/**
 * @file MemoryLoan.h
 * @brief テンプレートメモリプールクラスの実装
 * @author Gold Smith<br>
 * @date 2022-2024<br>
 * @copyright SPDX-License-Identifier: MIT<br>
 * Released under the MIT license<br>
 * https: //opensource.org/licenses/mit-license.php<br>
 * このファイル内のすべてのコードは、特に明記されていない限り、MITライセンスに従います。
 */

#pragma once
 /// @attention このクラスはWindows専用です。<br>
 /// 異常が検出された場合、必ず例外が発生します。

// ********使用条件を設定**************************

/// @def MR_USING_CRITICAL_SECTION
/// @brief クリティカルセクションを使用する場合
#define MR_USING_CRITICAL_SECTION

/// @def MR_CONFIRM_RANGE
/// @brief 範囲外確認が必要の場合。
#define MR_CONFIRM_RANGE

/// @def MR_USING_DEBUG_OUT
/// @brief デバッグ出力をする場合。
#define MR_USING_DEBUG_OUT

/// @def MR_USING_STD_ERROR
/// @brief std::cerr出力をする場合。
#define MR_USING_STD_ERROR

// **********条件設定終わり************************

#include <memory>
#include <memory_resource>
#include <string>
#include <exception>
#include <iostream>
#include <sstream>

#ifdef MR_USING_DEBUG_OUT
#include <debugapi.h>
#endif // MR_USING_DEBUG_OUT
#if defined(MR_CONFIRM_RANGE) || defined(MR_USING_DEBUG_OUT) || defined(MR_USING_STD_ERROR)
#include <algorithm>
#endif
#ifdef MR_USING_CRITICAL_SECTION
#include <synchapi.h>
#endif // MR_USING_CRITICAL_SECTION

template <typename T>
class MemoryLoan {
public:
	MemoryLoan() = delete;
	// @attention sizeInは2のべき乗で無くてはなりません。
	MemoryLoan(T *const pBufIn, size_t sizeIn)
		:ppBuf(nullptr)
		, front(0)
		, end(0)
#if defined(MR_CONFIRM_RANGE) || defined(MR_USING_DEBUG_OUT) || defined(MR_USING_STD_ERROR)
		, max_using(0)
#endif
		, mask(sizeIn - 1)
#ifdef MR_USING_CRITICAL_SECTION
		, cs{}
#endif // MR_USING_CRITICAL_SECTION

	{
		if ((sizeIn & (sizeIn - 1)) != 0) {
			std::string estr("The number of units for MemoryLoan must be specified as a power of 2.\r\n");
#ifdef MR_USING_DEBUG_OUT
			::OutputDebugStringA(estr.c_str());
#endif// MR_USING_DEBUG_OUT
#ifdef MR_USING_STD_ERROR
			std::cerr << estr;
#endif // MR_USING_STD_ERROR
			throw std::invalid_argument(estr);
		}
#ifdef MR_USING_CRITICAL_SECTION
		(void)::InitializeCriticalSection(&cs);
#endif // MR_USING_CRITICAL_SECTION
		ppBuf = new T * [sizeIn];
		for (size_t i(0); i < sizeIn; ++i) {
			ppBuf[i] = &pBufIn[i];
		}
	}
	MemoryLoan(const MemoryLoan &) = delete;
	MemoryLoan(const MemoryLoan &&)noexcept = delete;
	MemoryLoan &operator ()(const MemoryLoan &) = delete;
	MemoryLoan &operator =(const MemoryLoan &) = delete;
	MemoryLoan &operator ()(MemoryLoan &&) = delete;
	MemoryLoan &operator =(MemoryLoan &&) = delete;
	~MemoryLoan() {
#ifdef MR_USING_DEBUG_OUT
		std::stringstream ss;
		ss << "MemoryLoan is destructing."
			<< " DebugMessage:" << "\"" << strDebug << "\""
			<< " TypeName:" << "\"" << typeid(T).name() << "\""
			<< " BytesPerUnit:" << sizeof(T) << "bytes"
			<< " TotalNumberOfLoans:" << std::to_string(end)
			<< " TotalNumberOfReturns:" << std::to_string(front)
			<< " NumberOfUnreturned:" << std::to_string((int)(end - front))
			<< " NumberOfUnits:" << std::to_string(mask + 1)
			<< " MaximumNumberOfLoans:" << std::to_string(max_using)
			<< "\r\n";
		::OutputDebugStringA(ss.str().c_str());
#endif // MR_USING_DEBUG_OUT

#ifdef MR_USING_CRITICAL_SECTION
		::DeleteCriticalSection(&cs);
#endif // MR_USING_CRITICAL_SECTION
		delete[] ppBuf;
	}


	//sizeInは2のべき乗で無くてはなりません。
	void ReInitialized(T *pBufIn, size_t sizeIn) {
#ifdef MR_USING_DEBUG_OUT
		std::stringstream ss;
		ss << "MemoryLoan reinitialized."
			<< " DebugMessage:" << "\"" << strDebug << "\""
			<< " TypeName:" << "\"" << typeid(T).name() << "\""
			<< " BytesPerUnit:" << sizeof(T) << "bytes"
			<< " TotalNumberOfLoans:" << std::to_string(end)
			<< " TotalNumberOfReturns:" << std::to_string(front)
			<< " NumberOfUnreturned:" << std::to_string((int)(end - front))
			<< " NumberOfUnits:" << std::to_string(mask + 1)
			<< " MaximumNumberOfLoans:" << std::to_string(max_using)
			<< "\r\n";
		::OutputDebugStringA(ss.str().c_str());
#endif // MR_USING_DEBUG_OUT
		delete[] ppBuf;
		front = 0;
		end = 0;
		mask = sizeIn - 1;
		if ((sizeIn & (sizeIn - 1)) != 0) {
			std::string estr("The number of units for MemoryLoan must be specified as a power of 2.\r\n");
#ifdef MR_USING_DEBUG_OUT
			::OutputDebugStringA(estr.c_str());
#endif// MR_USING_DEBUG_OUT
#ifdef MR_USING_STD_ERROR
			std::cerr << estr;
#endif // MR_USING_STD_ERROR
			throw std::invalid_argument(estr);
		}
		ppBuf = new T * [sizeIn];
		for (size_t i(0); i < sizeIn; ++i) {
			ppBuf[i] = &pBufIn[i];
		}
	}

	inline T *Lend() {
#ifdef MR_USING_CRITICAL_SECTION
		std::unique_ptr< ::CRITICAL_SECTION, decltype(::LeaveCriticalSection) *> qcs
			= { [&]() {::EnterCriticalSection(&cs); return &cs; }(),::LeaveCriticalSection };
#endif // MR_USING_CRITICAL_SECTION
#ifdef MR_CONFIRM_RANGE
		if ((front + mask + 1) < (end + 1)) {
			std::stringstream ss;
			ss << __FILE__ << "(" << __LINE__ << "):"
				<< "Loans will soon surpass units."
				<< " DebugMessage:" << "\"" << strDebug << "\""
				<< " TypeName:" << "\"" << typeid(T).name() << "\""
				<< " BytesPerUnit:" << sizeof(T) << "bytes"
				<< " TotalNumberOfLoans:" << std::to_string(end)
				<< " TotalNumberOfReturns:" << std::to_string(front)
				<< " NumberOfUnreturned:" << std::to_string((int)(end - front))
				<< " NumberOfUnits:" << std::to_string(mask + 1)
				<< " MaximumNumberOfLoans:" << std::to_string(max_using)
				<< "\r\n";
#ifdef MR_USING_STD_ERROR
			std::cerr << ss.str();
#endif // MR_USING_STD_ERROR
#ifdef MR_USING_DEBUG_OUT
			::OutputDebugStringA(ss.str().c_str());
#endif// MR_USING_DEBUG_OUT
			throw std::out_of_range(ss.str().c_str()); // 例外送出
		}
#endif // MR_CONFIRM_RANGE
		T **ppT = &ppBuf[end & mask];
		++end;
#if defined(MR_CONFIRM_RANGE) || defined(MR_USING_DEBUG_OUT) || defined(MR_USING_STD_ERROR)
		max_using = std::max<size_t>(end - front, max_using);
#endif
		return *ppT;
	}

	inline void Return(T *const pT) {
#ifdef MR_USING_CRITICAL_SECTION
		std::unique_ptr< ::CRITICAL_SECTION, decltype(::LeaveCriticalSection) *> qcs
			= { [&]() {::EnterCriticalSection(&cs); return &cs; }(),::LeaveCriticalSection };
#endif // MR_USING_CRITICAL_SECTION
#ifdef MR_CONFIRM_RANGE
		if ((front + 1) > end) {
			std::stringstream ss;
			ss << __FILE__ << "(" << __LINE__ << "):"
				<< " Returns exceed loans."
				<< " DebugMessage:" << "\"" << strDebug << "\""
				<< " TypeName:" << "\"" << typeid(T).name() << "\""
				<< " BytesPerUnit:" << sizeof(T) << "bytes"
				<< " TotalNumberOfLoans:" << std::to_string(end)
				<< " TotalNumberOfReturns:" << std::to_string(front)
				<< " NumberOfUnreturned:" << std::to_string((int)(end - front))
				<< " NumberOfUnits:" << std::to_string(mask + 1)
				<< " MaximumNumberOfLoans:" << std::to_string(max_using)
				<< "\r\n";
#ifdef MR_USING_STD_ERROR
			std::cerr << ss.str();
#endif // MR_USING_STD_ERROR
#ifdef MR_USING_DEBUG_OUT
			::OutputDebugStringA(ss.str().c_str());
#endif// MR_USING_DEBUG_OUT
			throw std::out_of_range(ss.str().c_str()); // 例外送出
		}
#endif // MR_CONFIRM_RANGE
		ppBuf[front & mask] = pT;
		++front;
	}

	void DebugString(const std::string str) {
#if defined(MR_CONFIRM_RANGE) ||defined(MR_USING_DEBUG_OUT) || defined(MR_USING_STD_ERROR)
		strDebug = str;
#endif
	}

protected:
	T **ppBuf;
	size_t front;
	size_t end;
	size_t mask;
#if defined(MR_CONFIRM_RANGE) ||defined(MR_USING_DEBUG_OUT) || defined(MR_USING_STD_ERROR)
	size_t max_using;
	std::string strDebug;
#endif

#ifdef MR_USING_CRITICAL_SECTION
	::CRITICAL_SECTION cs;
#endif // MR_USING_CRITICAL_SECTION

};

#undef MR_USING_CRITICAL_SECTION
#undef MR_CONFIRM_RANGE
#undef MR_USING_DEBUG_OUT
#undef MR_USING_STD_ERROR
