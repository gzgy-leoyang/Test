#ifdef __cplusplus
extern "C"{
#endif

#ifndef MCU_CTRL_H
#define MCU_CTRL_H

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/file.h>

/*
 * @brief  串口接口初始配置，
 * @details 串口初始化，默认初始化"/dev/ttyS1"，波特率：115200 数据：8位 无奇偶校验 停止位：1位
 * @param
 * @return 0:初始化成功，-1：初始化失败
 * @note
 *
 */
extern int uart_init(void );

/*
 * @brief  串口去初始接口，
 * @details 串口去初始化，关闭打开的串口句柄
 * @param
 * @return 0:去初始化成功，-1：去初始化失败
 * @note
 *
 */
extern int uart_deinit(void );

/*
 * @brief 读取MCU所支持的最大波特率
 * @details
 * @param  int* 保存结果数据的指针
 * @return 0:返回有效
 * @note
 *
 */
extern int uart_get_bitrate( int* _bit );

/*
 * @brief 设置MCU波特率
 * @details
 * @param  int :波特率
 * @return 0:设置成功 -1:设置失败
 * @note
 *
 */
extern int uart_set_bitrate( int bitrate);

/*
 * @brief 获取发送数据最大数据长度
 * @details
 * @param  int :数据长度
 * @return 0:获取成功 -1:获取失败
 * @note
 */
extern int uart_get_sizeof_buffer( int* _size);

/*
 * @brief 获取stm32内部固件的版本信息
 * @details 字符串格式：YQ%i.%i.%iV%i
 * @param  char* 保存结果数据的指针
 * @return 0:获取成功 -1:获取失败
 * @note
 */
extern int uart_get_fireware_version( char* _str);

/*
 * @brief 获取硬件清单版本号
 * @details 字符串格式：V%i
 * @param  char* 保存结果数据的指针
 * @return 0:获取成功 -1:获取失败
 * @note
 */
extern int uart_get_hardware_version( char* _str);

/*
 * @brief 获取协处理器软件编译时间
 * @details 字符串格式：年-月-日 时:分:秒 
 * @param  char* 保存结果数据的指针
 * @return 0:获取成功 -1:获取失败
 * @note
 */
extern int uart_get_mcu_compile_time( char* _str);

/*
 * @brief 获取协处理器配置文件生成时间
 * @details 字符串格式：年-月-日 时:分:秒 
 * @param  char* 保存结果数据的指针
 * @return 0:获取成功 -1:获取失败
 * @note
 */
extern int uart_get_mcu_cfg_time( char* _str);

/*
 * @brief 获取ACC和B+电压值
 * @details 单位mV
 * @param  _acc :ACC P+电压 _bat:B+电压
 * @return 0:获取成功 -1:获取失败
 * @note
 */
extern int uart_get_acc_bat_voltage( int* _acc, int* _bat);

/*
 * @brief 设置电源关机
 * @details
 * @param  void
 * @return 0:设置成功 -1:设置失败
 * @note
 */
extern int uart_set_power_off_system();

/*
 * @brief 设置看门狗时间
 * @details 单位秒，如果不设置，默认为20秒，设为0代表立即重启系统，设为为0xffff时代表关闭看门狗
 * @param  _time:看门狗时间
 * @return 0:设置成功 -1:设置失败
 * @note
 */
extern int uart_set_watchdog_time(int _time);

/*
 * @brief 获取看门狗时间
 * @details 单位秒，如果不设置，默认为20秒，设为0代表立即重启系统，设为为0xffff时代表关闭看门狗
 * @param  *_time:看门狗时间数据指针
 * @return 0:设置成功 -1:设置失败
 * @note
 */
extern int uart_get_watchdog_time(int* _time);

/*
 * @brief 升级MCU固件和配置文件
 * @details 包含MCU固件和配置文件
 * @param  _file_name：固件文件路径 _cfg_name：配置文件路径
 * @return 0:升级成功 -1：升级失败
 * @note
 */
extern int uart_upgrade(const char* _file_name , const char* _cfg_name);

/*
 * @brief 获取PWM频率和占空比
 * @details
 * @param  _ch :通道 _freq：频率 _duty占空比
 * @return 0:获取成功 -1:获取失败
 * @note
 */
extern int uart_get_pwm( const int _ch, int* _freq, int* _duty);

/*
 * @brief 设置PWM频率和占空比
 * @details
 * @param  _ch :通道 _freq：频率 _duty占空比
 * @return 0:设置成功 -1:设置失败
 * @note
 */
extern int uart_set_pwm( const int _ch,const int _freq,const int _duty);

/*
 * @brief 设置开关输出
 * @details
 * @param  _ch :通道 _sw：开关量0输出低，1输出高
 * @return 0:设置成功 -1:设置失败
 * @note
 */
extern int uart_set_switch( const int _ch,const int _sw);

/*
 * @brief 获取直通口数据
 * @details
 * @param  _ch :通道 _val :直接口数据
 * @return 0:获取成功 -1:获取失败
 * @note
 */
extern int uart_get_in_ch( const int _ch,uint32_t* _val );

/*
 * @brief 获取扩展直通口数据
 * @details
 * @param  _ch :通道 _val :扩展直接口数据
 * @return 0:获取成功 -1:获取失败
 * @note
 */
extern int uart_get_extIn_ch( const int _ch,uint32_t* _val );

/*
 * @brief 获取频率数据
 * @details
 * @param  _ch :通道 _val :频率数据，获取到的频率需要除10为实际值
 * @return 0:获取成功 -1:获取失败
 * @note
 */
extern int uart_get_freq( const int _ch,uint32_t* _val );

/*
 * @brief 获取GPIO数值
 * @details
 * @param  _ch :通道 _val :GPIO数值
 * @return 0:获取成功 -1:获取失败
 * @note
 */
extern int uart_get_gpio( const int _ch, int* _val );

/*
 * @brief 设置GPIO数值
 * @details
 * @param  _ch :通道 _val :GPIO数值
 * @return 0:设置成功 -1:设置失败
 * @note
 */
extern int uart_set_gpio( const int _ch,const int _val );

/*
 * @brief 设置打印串口传输数据详情，默认关闭
 * @details
 * @param  sw :开关 0:闭 1:打开
 * @return 0:设置成功 -1:设置失败
 * @note
 */
extern int uart_set_print_data_detail( const uint8_t sw );

/*
 * @brief 设置打印串口传输设备号，如果不设置，默认为/dev/ttyS0，该接口必须在uart_init函数之前调用
 * @details
 * @param  dev :串口设备号
 * @return 0:设置成功 -1:设置失败
 * @note
 */
extern int uart_set_uart_dev( char* dev );

/*
 * @brief 设置MCU重启
 * @details 
 * @param  void
 * @return 0:设置成功 -1:设置失败
 * @note
 */
extern int uart_set_mcu_reboot();

/*
 * @brief 获取mcu库版本
 * @details
 * @param  _ver :保存库版本号字符串
 * @return 0:设置成功 -1:设置失败
 * @note
 */
extern int uart_get_lib_version( char* _ver );

#endif // MCU_CTRL_H

#ifdef __cplusplus
}
#endif
