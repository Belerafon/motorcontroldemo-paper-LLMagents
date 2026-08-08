/*!
     Copyright 2017 	АО "НИИЭТ" и ООО "НПФ ВЕКТОР"

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

 \file      V_IQmath.h
 \brief     Библиотека функций целочисленной математики
 \author    ООО "НПФ Вектор". Все права защищены. http://motorcontrol.ru
 \version   v 2.0 25/03/2016

 \defgroup  V_IQmath Библиотека функций целочисленной математики

 \addtogroup V_IQmath 
@{*/


#ifndef V_IQ_MATH_H
#define V_IQ_MATH_H



#ifdef __cplusplus
extern "C" {
#endif

#include "DSP.h"


static const int32 fix16_max = 0x7FFFFFFF; /*!< the maximum value of int32 */
static const int32 fix16_min = 0x80000000; /*!< the minimum value of int32 */
static const int32 fix16_overflow = 0x80000000; /*!< the value used to indicate overflows when FIXMATH_NO_OVERFLOW is not specified */


static const int32 fix16_pi  = 205887;
static const int32 fix16_e   = 178145;
static const int32 fix16_one = 0x00010000;
#define _PI 3.1415926535f
typedef int32 _iq;


//Преобразование данных в формате с плавающей точкой в целочисленные форматы от IQ1 до IQ30.
#define   _IQ30(A)      (long) ((A) * 1073741824.0f)
#define   _IQ29(A)      (long) ((A) * 536870912.0f)
#define   _IQ28(A)      (long) ((A) * 268435456.0f)
#define   _IQ27(A)      (long) ((A) * 134217728.0f)
#define   _IQ26(A)      (long) ((A) * 67108864.0f)
#define   _IQ25(A)      (long) ((A) * 33554432.0f)
#define   _IQ24(A)      (long) ((A) * 16777216.0f)
#define   _IQ23(A)      (long) ((A) * 8388608.0f)
#define   _IQ22(A)      (long) ((A) * 4194304.0f)
#define   _IQ21(A)      (long) ((A) * 2097152.0f)
#define   _IQ20(A)      (long) ((A) * 1048576.0f)
#define   _IQ19(A)      (long) ((A) * 524288.0f)
#define   _IQ18(A)      (long) ((A) * 262144.0f)
#define   _IQ17(A)      (long) ((A) * 131072.0f)
#define   _IQ16(A)      (long) ((A) * 65536.0f)
#define   _IQ15(A)      (long) ((A) * 32768.0f)
#define   _IQ14(A)      (long) ((A) * 16384.0f)
#define   _IQ13(A)      (long) ((A) * 8192.0f)
#define   _IQ12(A)      (long) ((A) * 4096.0f)
#define   _IQ11(A)      (long) ((A) * 2048.0f)
#define   _IQ10(A)      (long) ((A) * 1024.0f)
#define   _IQ9(A)       (long) ((A) * 512.0f)
#define   _IQ8(A)       (long) ((A) * 256.0f)
#define   _IQ7(A)       (long) ((A) * 128.0f)
#define   _IQ6(A)       (long) ((A) * 64.0f)
#define   _IQ5(A)       (long) ((A) * 32.0f)
#define   _IQ4(A)       (long) ((A) * 16.0f)
#define   _IQ3(A)       (long) ((A) * 8.0f)
#define   _IQ2(A)       (long) ((A) * 4.0f)
#define   _IQ1(A)       (long) ((A) * 2.0f)

//по умолчанию формат IQ24
#define   _IQ(A)  _IQ24(A)
#define   _IQmpy(A,B)  _IQ24mpy(A,B)
#define   _IQmpyI32(A,B)  _IQ24mpyI32(A,B)
#define   _IQdiv(A,B)  _IQ24div(A,B)
#define   _IQsqrt(A) _IQ24sqrt(A)
#define   _IQsinPU(A) _IQ24sinPU(A)
#define   _IQcosPU(A) _IQ24cosPU(A)
#define   _IQsin(A) _IQ24sin(A)
#define   _IQcos(A) _IQ24cos(A)
#define   _IQtanPU(A) _IQ24tanPU(A)
#define   _IQatan2PU(A,B) _IQ24atan2PU(A,B)
#define   _IQmag(A,B)  _IQ24mag(A,B)
#define   _IQtoF(A)  _IQ24toF(A)


int32 _IQ24sinPU(int32 inAngle);
int32 _IQ24atan2PU(int32 inY , int32 inX);
int32 _IQ24sinPU_accurate(int32 inAngle);
int32 _IQ24sqrt(int32 inValue);
int32 _IQ16div(int32 a, int32 b);
int32 _IQ24div(int32 a, int32 b);
int32 _IQ21div(int32 a, int32 b);
int32 _IQ10div(int32 a, int32 b);
int32 _IQ24mag(int32 a, int32 b);

//! Преобразует из целочисленного 8.24 в формат float
//! \memberof V_IQmath
#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline float _IQ24toF(int32_t a) {
    return (float)(a)*(1.0/16777216.0f);
}

//! \memberof V_IQmath
#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline float _IQ21toF(int32_t a) {
    return (float)(a)*(1.0/2097152.0f);
}

//! \memberof V_IQmath
#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline float _IQ12toF(int32_t a) {
    return (float)(a)*(1.0/4096.0f);
}

//! Преобразует из целочисленного 22.10 в формат float

//! \memberof V_IQmath
#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline float _IQ10toF(int32_t a) {
    return (float)(a)*(1.0/1024.0f);
}

//! Преобразует из целочисленного 26.6 в формат float

//! \memberof V_IQmath
#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline float _IQ6toF(int32_t a) {
    return (float)(a)*(1.0/64.0f);
}

//! Умножение двух чисел в формате 8.24

//! \memberof V_IQmath
#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline int32 _IQ24mpy(int32 inArg0, int32 inArg1) {
    return (int32)(((int64_t)inArg0 * inArg1)>> 24);
}

//! \memberof V_IQmath
#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline int32 _IQ18mpy(int32 inArg0, int32 inArg1) {
    return (int32)(((int64_t)inArg0 * inArg1)>> 18);
}


//! \memberof V_IQmath
#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline int32 _IQ20mpy(int32 inArg0, int32 inArg1) {
    return (int32)(((int64_t)inArg0 * inArg1)>> 20);
}



//! Умножение двух чисел в формате 32.0

//! \memberof V_IQmath
#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline int32 _IQ24mpyI32(int32 inArg0, int32 inArg1) {
    return (inArg0 * inArg1);
}

//! Умножение двух чисел в формате 16.16

//! \memberof V_IQmath
#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline int32 _IQ16mpy(int32 inArg0, int32 inArg1) {
    int64_t product = (int64_t)inArg0 * inArg1;
    int32 result = product >> 16;
//  result += (product & 0x8000) >> 15; //Непонтяно для чего. Написано для округления
    return result;
}

//! Умножение двух чисел в формате 28.4

//! \memberof V_IQmath
#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline int32 _IQ4mpy(int32 inArg0, int32 inArg1) {
    return (int32)(((int64_t)inArg0 * inArg1)>> 4);
}


//! Функция косинуса, вызывает функцию синуса

//! \memberof V_IQmath
#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline int32 _IQ24cosPU(int32 inAngle) {
    return _IQ24sinPU(inAngle + _IQ24(0.25));
}

//! \memberof V_IQmath
#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline int32 _IQ18sinPU(int32 inAngle) {
    return _IQ24sinPU(inAngle<<6)>>6;
}


//! \memberof V_IQmath
#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline int32 _IQ18cosPU(int32 inAngle) {
    return _IQ24sinPU((inAngle<<6) + _IQ24(0.25))>>6;
}


//! Функция тангенса, вызывает функции синуса и косинуса.

//! \memberof V_IQmath
#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline int32 _IQ24tanPU(int32 inAngle) {
    return _IQ24div(_IQ24sinPU(inAngle), _IQ24sinPU(inAngle));
}

#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline int32 _IQ16toIQ(int32 a){
	return (a << 8);
}

#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline int32 _IQ24cos(int32 x){
	x = _IQmpy(x, _IQ(0.15915494309189533576));
	return _IQsinPU(x + _IQ24(0.25));
}

#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline int32 _IQ24sin(int32 x){
	x = _IQmpy(x, _IQ(0.15915494309189533576));
	return _IQsinPU(x);
}

#if defined (__GNUC__)
__attribute__((always_inline))
#elif defined (__CMCPPARM__)
__STATIC_INLINE
#endif
inline int32 _IQabs(int32 x){
	if (x<0)
		return -x;
	return x;
}

#ifdef __CMCPPARM__
#define fabs(x) fabsf(x)
#endif /* __CMCPPARM__ */



#ifdef __cplusplus
}
#endif

#endif

/*@}*/
