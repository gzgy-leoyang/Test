//thread_pool.h
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <mutex>
#include <queue>
#include <functional>
#include <future>
#include <thread>
#include <utility>
#include <vector>

#include <iostream>

////////////
// 任务队列类
////////////
template <typename T>
class SafeQueue
{
private:
  // 任务队列,由线程队列中被唤醒的线程查询任务队列顶部取出任务
  std::queue<T> m_queue;
  // 互斥信号,防止多个线程同时操作任务队列
  std::mutex m_mutex; 
public:
  // 构造和析构
  SafeQueue() {}
  SafeQueue(SafeQueue &&other) {}
  ~SafeQueue() {}

  // 查询当前任务队列,对std::queue的同名方法封装一次,提供互斥
  bool empty()
  {
    // 在入口处,请求互斥锁,由于是局部的,
    // 在离开当前范围后,在锁的析构函数中释放锁
    std::unique_lock<std::mutex> lock(m_mutex);
    return m_queue.empty();
  }

  // 同上
  long unsigned int size()
  {
    std::unique_lock<std::mutex> lock(m_mutex); 
    return m_queue.size();
  }

  // 互斥过程同上,向任务队列尾部添加新的任务,也就是外部需要执行的函数指针
  void enqueue(T &t)
  {
    // 8.向队列加入任务函数
    std::unique_lock<std::mutex> lock(m_mutex);
    m_queue.emplace(t);
  }

  // 同样的互斥过程,从队列取出任务并通过 ref 方式导出,
  // 由当前被唤醒的空闲的线程装载执行
  bool dequeue(T &t)
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_queue.empty())
        return false;
    // 取出顶部的任务对象
    t = std::move(m_queue.front());
    // 将顶部任务弹出(删除)
    m_queue.pop();
    return true;
  }
};

////////////
// 线程池类
////////////
class ThreadPool
{
private:
    class ThreadWorker // 内置线程工作类
    {
    private:
      ThreadPool *m_pool; // 所属线程池
      int         m_id;   // 工作id
    public:
      // 构造函数
      ThreadWorker(ThreadPool *pool, const int id) : 
        m_pool(pool),
        m_id(id){}

      // 重载()操作
      void operator()()
      {
        std::function<void()> func; // 定义基础函数类func
        
        // 只要没有关闭线程池,则一直在此打转,
        // 只是由于wait唤醒,实际上是阻塞在此
        while (!m_pool->m_shutdown){
          bool dequeued;              // 是否正在取出队列中元素
          // 加括号,只是用于划定互斥锁的范围
          {
            // 为线程环境加锁，互访问工作线程的休眠和唤醒
            std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);
            // 如果任务队列为空，阻塞当前线程
            // 直到有任务函数被压入任务队列,
            // 并主动唤醒线程队列中的一个线程
            // 2.1 线程建立后,实际阻塞于此
            if (m_pool->m_queue.empty()){
              m_pool->m_conditional_lock.wait(lock); // 等待条件变量通知，开启线程
            }
            // 10. 线程被唤醒后,主动查询任务队列,取一个执行,完成后再次进入等待状态
            // 在唤醒线程之前,都在以上部分等待,被唤醒后执行以下部分
            // 取出任务队列中的元素
            dequeued = m_pool->m_queue.dequeue(func);
          }
          // 如果成功取出，执行工作函数
          if (dequeued) {
            // 11. 实际执行任务函数
            func();
          }
        }
      }
    };

    // 线程池成员
    std::vector<std::thread>         m_threads;           // 工作线程队列
    bool                             m_shutdown;          // 线程池是否关闭
    SafeQueue<std::function<void()>> m_queue;             // 执行函数安全队列，即任务队列
    std::mutex                       m_conditional_mutex; // 线程休眠锁互斥变量
    std::condition_variable          m_conditional_lock;  // 线程环境锁，可以让线程处于休眠或者唤醒状态
public:
    // ///////////////////////////////
    // 构造线程池
    // 生产一个包含指定个数的线程的队列(不包含实际对象,仅是长度满足要求),
    // 之后通过init初始化每一线程
    explicit ThreadPool(const int n_threads = 4)
        : m_threads(std::vector<std::thread>(n_threads)), 
          m_shutdown(false){ 
      // 1. 新建线程池
    }


    // C++11 特性,用=delete修饰拷贝构造函数和拷贝赋值函数
    // 不能被实例调用(编译器默认会提供默认的拷贝和拷贝赋值函数)
    // 经过如下处理,实际上就是禁止该类对象实例进行拷贝和拷贝赋值进行构造
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;
    // Inits thread pool
    // ///////////////////////////////
    // 向队列中的线程对象添加实际的线程
    void init()
    {
      for (long unsigned int i = 0; i < m_threads.size(); ++i){
        // 2. 在线程池中添加线程,由于没有工作任务,添加完成后立即阻塞在等待状态
          m_threads.at(i) = std::thread(ThreadWorker(
                                                this, 
                                                i)
                                          );
      }
    }

    // ///////////////////////////////
    // 关闭线程池
    void shutdown()
    {
      m_shutdown = true;
      // 唤醒所有的线程(应该都是空闲的)
      m_conditional_lock.notify_all(); 
      for ( long unsigned int i = 0; i < m_threads.size(); ++i){
        // 判断当前线程是否属于可以释放的?
        if (m_threads.at(i).joinable()){
          // 当线程空闲时,主动释放该线程资源
          m_threads.at(i).join();
        }
      }
    }

    // ///////////////////////////////
    // 3. 向线程池压入任务,之后立即从阻塞等待的线程中唤醒一个
    // 向线程池添加任务
    // 注1:使用"尾返回类型推导",格式: auto xxxx() -> decltype(函数调用)
    // 在编译时执行推导,则该运算符的结果=被调用函数的返回值类型,
    /* 如下模板函数,声明的返回值类型就是根据decltype()推导的类型,与计算结果
     * 天然就是一致的
     *template<typename T, typename U>
      auto add2(T x, U y) -> decltype(x+y){
        return x + y;
      }
     */ 
    // 注:std::funciton函数包装器,可以包装函数,函数指针,成员函数,
    // 静态函数,lamda表达式和函数对象
    // 注:在模板编程中,typename与class关键字类似,向编译器声明后续字符串为类型名称
    template <typename F, typename... Args>
    auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
    {
      // 3. 压入任务函数
      // 4. 通过绑定和传值,包装一个func实例
      // 连接函数和参数定义，特殊函数类型，避免左右值错误
      std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), 
                                                             std::forward<Args>(args)...); 

      // 5. 将 func 实例包装为一个auto类型的 task_ptr,该指针后续用于异步返回结果
      // 注: auto 共享指针=make_shared<packaged_task<decltype推导返回类型(空)>>(参数..)
      auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
      // 注:std::packaged_task 包装一个可调用的对象，并且允许异步获取
      // 该可调用对象产生的结果(返回值传给future)
      // std::packaged_task<函数返回类型(参数类型)> 变量名{函数名}
      // 注: 通过make_shared函数创建共享指针,返回一个指向该类型对象的共享指针
      // std::shared_ptr<T> make_shared< T >( Args&&... args );

      // Warp packaged task into void function
      // 6. 将已经包装为auto的函数,进一步包装为 warpper_func 函数
      std::function<void()> warpper_func=[task_ptr](){ (*task_ptr)(); };
      // 7. 将包装的函数压入任务队列中
      m_queue.enqueue(warpper_func);
      // 9. 唤醒一个等待中的线程,该线程唤醒后会主动从任务队列顶部获取任务并执行
      m_conditional_lock.notify_one();
      // 12.在任务执行完成后,在此返回任务的结果
      return task_ptr->get_future();
    }
};
#endif
