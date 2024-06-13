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

// ********使用条件を設定***********

/// @def USING_CRITICAL_SECTION
/// @brief クリティカルセクションを使用する場合
//#define USING_CRITICAL_SECTION

/// @def CONFIRM_RANGE
/// @brief 範囲外確認が必要の場合。
#define CONFIRM_RANGE

/// @def USING_STD_ERROR
/// @brief std::cerr出力をする場合。
//#define USING_STD_ERROR

/// @def USING_DEBUG_STRING
/// @brief デバッグ出力をする場合。
#define USING_DEBUG_STRING

// ******条件設定終わり*************

#ifdef USING_DEBUG_STRING
#include <debugapi.h>
#include <algorithm>
#include <string>
#endif // USING_DEBUG_STRING

#ifdef USING_CRITICAL_SECTION
#include <synchapi.h>
#endif // USING_CRITICAL_SECTION

#include <exception>
#include <iostream>
#include <sstream>

template <class T>class MemoryLoan
{
public:
	
	
	MemoryLoan() = delete;
	//sizeInは2のべき乗で無くてはなりません。
	MemoryLoan(T* const pBufIn, size_t sizeIn)
		:ppBuf(nullptr)
		, size(sizeIn)
		, front(0)
		, end(0)
#ifdef CONFIRM_RANGE
		, max_using(0)
#endif // CONFIRM_POINT
		, mask(sizeIn - 1)
#ifdef USING_CRITICAL_SECTION
		, cs{}
#endif // USING_CRITICAL_SECTION
	{
		if ((sizeIn & (sizeIn - 1)) != 0)
		{
			std::string estr("Err! The MemoryLoan must be Power of Two.\r\n");
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(estr.c_str());
#endif// USING_DEBUG_STRING
#ifdef USING_STD_ERROR
			std::cerr << estr;
#endif // USING_STD_ERROR
			throw std::invalid_argument(estr);
		}
#ifdef USING_CRITICAL_SECTION
		(void)InitializeCriticalSection(&cs);
#endif // USING_CRITICAL_SECTION
		ppBuf = new T * [sizeIn];
		for (size_t i(0); i < size; ++i)
		{
			ppBuf[i] = &pBufIn[i];
		}
	}
	MemoryLoan(const MemoryLoan&) = delete;
	MemoryLoan(const MemoryLoan&&)noexcept = delete;
	MemoryLoan& operator ()(const MemoryLoan&) = delete;
	MemoryLoan& operator =(const MemoryLoan&) = delete;
	MemoryLoan& operator ()(MemoryLoan&&) = delete;
	MemoryLoan& operator =(MemoryLoan&&) = delete;
	~MemoryLoan()
	{
#ifdef USING_CRITICAL_SECTION
		DeleteCriticalSection(&cs);
#endif // USING_CRITICAL_SECTION

#ifdef USING_DEBUG_STRING
		std::stringstream ss;
		ss << "MemoryLoan "
			<< "DebugMessage:"
			<< "\"" << strDebug.c_str() << "\" "
			<< "TypeName:"<<"\""<< typeid(T).name() << "\" "
			<< "MemorySizeOfTheUnit:"<< sizeof(T)<<"bytes "
			<< "TotalLoans:" << std::to_string(front)
			<< " TotalReturns:" << std::to_string(end)
			<< " NotReturned:" << std::to_string((long long)end - (long long)front)
			<< " NumberOfUnits:" << std::to_string(size)
			<< " MaximumPeakLoans:" << std::to_string(max_using)
			<< "\r\n";
		OutputDebugStringA(ss.str().c_str());
#endif // USING_DEBUG_STRING
		delete[]ppBuf;
	}

	//sizeInは2のべき乗で無くてはなりません。
	void ReInitialize(T* pBufIn, size_t sizeIn)
	{
		delete[]ppBuf;
		ppBuf = nullptr;
		size = sizeIn;
		front = 0;
		end = 0;
		const_cast<size_t>(mask) = sizeIn - 1;

		if ((sizeIn & (sizeIn - 1)) != 0)
		{
			std::string estr("Err! The MemoryLoan must be Power of Two.\r\n");
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(estr.c_str());
#endif// USING_DEBUG_STRING
#ifdef USING_STD_ERROR
			std::cerr << estr;
#endif // USING_STD_ERROR
			throw std::invalid_argument(estr);
		}

		ppBuf = new T * [sizeIn];

		for (size_t i(0); i < size; ++i)
		{
			ppBuf[i] = &pBufIn[i];
		}
	}

	inline T* Lend()
	{
#ifdef USING_CRITICAL_SECTION
		std::unique_ptr< CRITICAL_SECTION, void(*)(CRITICAL_SECTION*)> qcs
			= { [&]() {EnterCriticalSection(&cs); return &cs; }()
			,[](CRITICAL_SECTION* pcs) {LeaveCriticalSection(pcs); } };
#endif // USING_CRITICAL_SECTION
#ifdef CONFIRM_RANGE
		if (front + size < end)
		{
			std::stringstream ss;
			ss << __FILE__ << "(" << __LINE__ << "): "
				<< "Units have been returned more than rentals. "
#ifdef USING_DEBUG_STRING
				<< "DebugMessage:"
				<< "\"" << strDebug << "\"" << " "
#endif// USING_DEBUG_STRING
				<< "TypeName:" << "\"" << typeid(T).name() << "\" "
				<< "MemorySizeOfTheUnit:" << sizeof(T) << " "
				<< "TotalLoans:" << std::to_string(front)
				<< " TotalReturns:" << std::to_string(end)
				<< " NotReturned:" << std::to_string((long long)end - (long long)front)
				<< " NumberOfUnits:" << std::to_string(size)
				<< " MaximumPeakLoans:" << std::to_string(max_using)
				<< "\r\n";
#ifdef USING_STD_ERROR
			std::cerr << ss.str();
#endif // USING_STD_ERROR
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(ss.str().c_str());
#endif// USING_DEBUG_STRING
			throw std::out_of_range(ss.str().c_str()); // 例外送出
		}
#endif // !CONFIRM_POINT
		T** ppT = &ppBuf[end & mask];
		++end;
#ifdef CONFIRM_RANGE
		max_using=std::max<size_t>(end - front, max_using);
#endif // CONFIRM_POINT
		return *ppT;
	}

	inline void Return(T* pT)
	{
#ifdef USING_CRITICAL_SECTION
		std::unique_ptr< CRITICAL_SECTION, void(*)(CRITICAL_SECTION*)> qcs
			= { [&]() {EnterCriticalSection(&cs); return &cs; }()
			,[](CRITICAL_SECTION* pcs) {LeaveCriticalSection(pcs); }};
#endif // USING_CRITICAL_SECTION
#ifdef CONFIRM_RANGE
		if (front == end + size)
		{
			std::stringstream ss;
			ss << __FILE__ << "(" << __LINE__ << "): " 
				<< "Over the maximum number of units. "

#ifdef USING_DEBUG_STRING
				<< "DebugMessage:"
				<< "\"" << strDebug << "\"" << " "
#endif// USING_DEBUG_STRING
				<< "TypeName:" << "\"" << typeid(T).name() << "\" "
				<< "MemorySizeOfTheUnit:" << sizeof(T) << " "
				<< "TotalLoans:" << std::to_string(front)
				<< " TotalReturns:" << std::to_string(end)
				<< " NotReturned:" << std::to_string((long long)end - (long long)front)
				<< " NumberOfUnits:" << std::to_string(size)
				<< " MaximumPeakLoans:" << std::to_string(max_using)
				<< "\r\n";
#ifdef USING_STD_ERROR
			std::cerr << ss.str();
#endif // USING_STD_ERROR
#ifdef USING_DEBUG_STRING
			::OutputDebugStringA(ss.str().c_str());
#endif// USING_DEBUG_STRING
			throw std::out_of_range(ss.str()); // 例外送出
		}

#endif // !CONFIRM_POINT
		ppBuf[front & mask] = pT;
		++front;
	}

	void DebugString(const std::string str)
	{
#ifdef USING_DEBUG_STRING
		strDebug = str;
#endif // USING_DEBUG_STRING
	}

protected:
	T * *ppBuf; 
	size_t size;
	size_t front;
	size_t end;
	const size_t mask;
#ifdef USING_CRITICAL_SECTION
	CRITICAL_SECTION cs;
#endif // USING_CRITICAL_SECTION

#ifdef CONFIRM_RANGE
	size_t max_using;
#endif // CONFIRM_POINT
#ifdef USING_DEBUG_STRING
	std::string strDebug;
#endif // USING_DEBUG_STRING
};


//#ifdef USING_CRITICAL_SECTION
#undef USING_CRITICAL_SECTION
//#endif // USING_CRITICAL_SECTION

//#ifdef CONFIRM_RANGE
#undef CONFIRM_POINT
//#endif // CONFIRM_POINT

//#ifdef USING_DEBUG_STRING
#undef USING_DEBUG_STRING
//#endif // USING_DEBUG_STRING

//#ifdef USING_STD_ERROR
#undef USING_STD_ERROR
//#endif // USING_STD_ERROR
