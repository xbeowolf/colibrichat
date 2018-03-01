/******************************************************************************
*                                                                             *
* FastDelegateList.h -- delegates containers for native C++                   *
*                                                                             *
* Copyright (c) Podobashev Dmitry / BEOWOLF, 2009. All rights reserved.       *
*                                                                             *
******************************************************************************/

#ifndef FASTDELEGATELIST_H
#define FASTDELEGATELIST_H
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace fastdelegate {

	//N=0
	template<class RetType=detail::DefaultVoid>
	class FastDelegateList0 {
	public:
		typedef FastDelegate0<RetType> FastDelegate;
#ifdef FASTDELEGATE_LISTASVECTOR
		typedef std::vector<FastDelegate> List;
#else
		typedef std::list<FastDelegate> List;
#endif

		// Add delegate
		void operator += (const FastDelegate &x) {
			m_list.push_back(x);
		}
		// Substract delegate
		void operator -= (const FastDelegate &x) {
#ifdef FASTDELEGATE_LISTREMOVE
			m_list.remove(x);
#else
			List::iterator iter = std::find(m_list.begin(), m_list.end(), x);
			if (iter != m_list.end()) m_list.erase(iter);
#endif
		}
		// Invoke all delegates
		void operator() () const {
			for (List::const_iterator iter = m_list.begin(); iter != m_list.end(); iter++) {
				(*iter)(); }
		}
		// Safe invoke all delegates if list can modifies during call
		void Invoke() const {
			List list = m_list;
			for (List::const_iterator iter = list.begin(); iter != list.end(); iter++) {
				(*iter)(); }
		}
		// List operations
		void clear() { m_list.clear(); }
		const List& getlist() const { return m_list; }

	protected:
		List m_list;
	};

	//N=1
	template<class Param1, class RetType=detail::DefaultVoid>
	class FastDelegateList1 {
	public:
		typedef FastDelegate1<Param1, RetType> FastDelegate;
#ifdef FASTDELEGATE_LISTASVECTOR
		typedef std::vector<FastDelegate> List;
#else
		typedef std::list<FastDelegate> List;
#endif

		// Add delegate
		void operator += (const FastDelegate &x) {
			m_list.push_back(x);
		}
		// Substract delegate
		void operator -= (const FastDelegate &x) {
#ifdef FASTDELEGATE_LISTREMOVE
			m_list.remove(x);
#else
			List::iterator iter = std::find(m_list.begin(), m_list.end(), x);
			if (iter != m_list.end()) m_list.erase(iter);
#endif
		}
		// Invoke all delegates
		void operator() (Param1 p1) const {
			for (List::const_iterator iter = m_list.begin(); iter != m_list.end(); iter++) {
				(*iter)(p1); }
		}
		// Safe invoke all delegates if list can modifies during call
		void Invoke(Param1 p1) const {
			List list = m_list;
			for (List::const_iterator iter = list.begin(); iter != list.end(); iter++) {
				(*iter)(p1); }
		}
		// List operations
		void clear() { m_list.clear(); }
		const List& getlist() const { return m_list; }

	protected:
		List m_list;
	};

	//N=2
	template<class Param1, class Param2, class RetType=detail::DefaultVoid>
	class FastDelegateList2 {
	public:
		typedef FastDelegate2<Param1, Param2, RetType> FastDelegate;
#ifdef FASTDELEGATE_LISTASVECTOR
		typedef std::vector<FastDelegate> List;
#else
		typedef std::list<FastDelegate> List;
#endif

		// Add delegate
		void operator += (const FastDelegate &x) {
			m_list.push_back(x);
		}
		// Substract delegate
		void operator -= (const FastDelegate &x) {
#ifdef FASTDELEGATE_LISTREMOVE
			m_list.remove(x);
#else
			List::iterator iter = std::find(m_list.begin(), m_list.end(), x);
			if (iter != m_list.end()) m_list.erase(iter);
#endif
		}
		// Invoke all delegates
		void operator() (Param1 p1, Param2 p2) const {
			for (List::const_iterator iter = m_list.begin(); iter != m_list.end(); iter++) {
				(*iter)(p1, p2); }
		}
		// Safe invoke all delegates if list can modifies during call
		void Invoke(Param1 p1, Param2 p2) const {
			List list = m_list;
			for (List::const_iterator iter = list.begin(); iter != list.end(); iter++) {
				(*iter)(p1, p2); }
		}
		// List operations
		void clear() { m_list.clear(); }
		const List& getlist() const { return m_list; }

	protected:
		List m_list;
	};

	//N=3
	template<class Param1, class Param2, class Param3, class RetType=detail::DefaultVoid>
	class FastDelegateList3 {
	public:
		typedef FastDelegate3<Param1, Param2, Param3, RetType> FastDelegate;
#ifdef FASTDELEGATE_LISTASVECTOR
		typedef std::vector<FastDelegate> List;
#else
		typedef std::list<FastDelegate> List;
#endif

		// Add delegate
		void operator += (const FastDelegate &x) {
			m_list.push_back(x);
		}
		// Substract delegate
		void operator -= (const FastDelegate &x) {
#ifdef FASTDELEGATE_LISTREMOVE
			m_list.remove(x);
#else
			List::iterator iter = std::find(m_list.begin(), m_list.end(), x);
			if (iter != m_list.end()) m_list.erase(iter);
#endif
		}
		// Invoke all delegates
		void operator() (Param1 p1, Param2 p2, Param3 p3) const {
			for (List::const_iterator iter = m_list.begin(); iter != m_list.end(); iter++) {
				(*iter)(p1, p2, p3); }
		}
		// Safe invoke all delegates if list can modifies during call
		void Invoke(Param1 p1, Param2 p2, Param3 p3) const {
			List list = m_list;
			for (List::const_iterator iter = list.begin(); iter != list.end(); iter++) {
				(*iter)(p1, p2, p3); }
		}
		// List operations
		void clear() { m_list.clear(); }
		const List& getlist() const { return m_list; }

	protected:
		List m_list;
	};

	//N=4
	template<class Param1, class Param2, class Param3, class Param4, class RetType=detail::DefaultVoid>
	class FastDelegateList4 {
	public:
		typedef FastDelegate4<Param1, Param2, Param3, Param4, RetType> FastDelegate;
#ifdef FASTDELEGATE_LISTASVECTOR
		typedef std::vector<FastDelegate> List;
#else
		typedef std::list<FastDelegate> List;
#endif

		// Add delegate
		void operator += (const FastDelegate &x) {
			m_list.push_back(x);
		}
		// Substract delegate
		void operator -= (const FastDelegate &x) {
#ifdef FASTDELEGATE_LISTREMOVE
			m_list.remove(x);
#else
			List::iterator iter = std::find(m_list.begin(), m_list.end(), x);
			if (iter != m_list.end()) m_list.erase(iter);
#endif
		}
		// Invoke all delegates
		void operator() (Param1 p1, Param2 p2, Param3 p3, Param4 p4) const {
			for (List::const_iterator iter = m_list.begin(); iter != m_list.end(); iter++) {
				(*iter)(p1, p2, p3, p4); }
		}
		// Safe invoke all delegates if list can modifies during call
		void Invoke(Param1 p1, Param2 p2, Param3 p3, Param4 p4) const {
			List list = m_list;
			for (List::const_iterator iter = list.begin(); iter != list.end(); iter++) {
				(*iter)(p1, p2, p3, p4); }
		}
		// List operations
		void clear() { m_list.clear(); }
		const List& getlist() const { return m_list; }

	protected:
		List m_list;
	};

	//N=5
	template<class Param1, class Param2, class Param3, class Param4, class Param5, class RetType=detail::DefaultVoid>
	class FastDelegateList5 {
	public:
		typedef FastDelegate5<Param1, Param2, Param3, Param4, Param5, RetType> FastDelegate;
#ifdef FASTDELEGATE_LISTASVECTOR
		typedef std::vector<FastDelegate> List;
#else
		typedef std::list<FastDelegate> List;
#endif

		// Add delegate
		void operator += (const FastDelegate &x) {
			m_list.push_back(x);
		}
		// Substract delegate
		void operator -= (const FastDelegate &x) {
#ifdef FASTDELEGATE_LISTREMOVE
			m_list.remove(x);
#else
			List::iterator iter = std::find(m_list.begin(), m_list.end(), x);
			if (iter != m_list.end()) m_list.erase(iter);
#endif
		}
		// Invoke all delegates
		void operator() (Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const {
			for (List::const_iterator iter = m_list.begin(); iter != m_list.end(); iter++) {
				(*iter)(p1, p2, p3, p4, p5); }
		}
		// Safe invoke all delegates if list can modifies during call
		void Invoke(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const {
			List list = m_list;
			for (List::const_iterator iter = list.begin(); iter != list.end(); iter++) {
				(*iter)(p1, p2, p3, p4, p5); }
		}
		// List operations
		void clear() { m_list.clear(); }
		const List& getlist() const { return m_list; }

	protected:
		List m_list;
	};

	//N=6
	template<class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class RetType=detail::DefaultVoid>
	class FastDelegateList6 {
	public:
		typedef FastDelegate6<Param1, Param2, Param3, Param4, Param5, Param6, RetType> FastDelegate;
#ifdef FASTDELEGATE_LISTASVECTOR
		typedef std::vector<FastDelegate> List;
#else
		typedef std::list<FastDelegate> List;
#endif

		// Add delegate
		void operator += (const FastDelegate &x) {
			m_list.push_back(x);
		}
		// Substract delegate
		void operator -= (const FastDelegate &x) {
#ifdef FASTDELEGATE_LISTREMOVE
			m_list.remove(x);
#else
			List::iterator iter = std::find(m_list.begin(), m_list.end(), x);
			if (iter != m_list.end()) m_list.erase(iter);
#endif
		}
		// Invoke all delegates
		void operator() (Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6) const {
			for (List::const_iterator iter = m_list.begin(); iter != m_list.end(); iter++) {
				(*iter)(p1, p2, p3, p4, p5, p6); }
		}
		// Safe invoke all delegates if list can modifies during call
		void Invoke(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6) const {
			List list = m_list;
			for (List::const_iterator iter = list.begin(); iter != list.end(); iter++) {
				(*iter)(p1, p2, p3, p4, p5, p6); }
		}
		// List operations
		void clear() { m_list.clear(); }
		const List& getlist() const { return m_list; }

	protected:
		List m_list;
	};

	//N=7
	template<class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class RetType=detail::DefaultVoid>
	class FastDelegateList7 {
	public:
		typedef FastDelegate7<Param1, Param2, Param3, Param4, Param5, Param6, Param7, RetType> FastDelegate;
#ifdef FASTDELEGATE_LISTASVECTOR
		typedef std::vector<FastDelegate> List;
#else
		typedef std::list<FastDelegate> List;
#endif

		// Add delegate
		void operator += (const FastDelegate &x) {
			m_list.push_back(x);
		}
		// Substract delegate
		void operator -= (const FastDelegate &x) {
#ifdef FASTDELEGATE_LISTREMOVE
			m_list.remove(x);
#else
			List::iterator iter = std::find(m_list.begin(), m_list.end(), x);
			if (iter != m_list.end()) m_list.erase(iter);
#endif
		}
		// Invoke all delegates
		void operator() (Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7) const {
			for (List::const_iterator iter = m_list.begin(); iter != m_list.end(); iter++) {
				(*iter)(p1, p2, p3, p4, p5, p6, p7); }
		}
		// Safe invoke all delegates if list can modifies during call
		void Invoke(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7) const {
			List list = m_list;
			for (List::const_iterator iter = list.begin(); iter != list.end(); iter++) {
				(*iter)(p1, p2, p3, p4, p5, p6, p7); }
		}
		// List operations
		void clear() { m_list.clear(); }
		const List& getlist() const { return m_list; }

	protected:
		List m_list;
	};

	//N=8
	template<class Param1, class Param2, class Param3, class Param4, class Param5, class Param6, class Param7, class Param8, class RetType=detail::DefaultVoid>
	class FastDelegateList8 {
	public:
		typedef FastDelegate8<Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8, RetType> FastDelegate;
#ifdef FASTDELEGATE_LISTASVECTOR
		typedef std::vector<FastDelegate> List;
#else
		typedef std::list<FastDelegate> List;
#endif

		// Add delegate
		void operator += (const FastDelegate &x) {
			m_list.push_back(x);
		}
		// Substract delegate
		void operator -= (const FastDelegate &x) {
#ifdef FASTDELEGATE_LISTREMOVE
			m_list.remove(x);
#else
			List::iterator iter = std::find(m_list.begin(), m_list.end(), x);
			if (iter != m_list.end()) m_list.erase(iter);
#endif
		}
		// Invoke all delegates
		void operator() (Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8) const {
			for (List::const_iterator iter = m_list.begin(); iter != m_list.end(); iter++) {
				(*iter)(p1, p2, p3, p4, p5, p6, p7, p8); }
		}
		// Safe invoke all delegates if list can modifies during call
		void Invoke(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8) const {
			List list = m_list;
			for (List::const_iterator iter = list.begin(); iter != list.end(); iter++) {
				(*iter)(p1, p2, p3, p4, p5, p6, p7, p8); }
		}
		// List operations
		void clear() { m_list.clear(); }
		const List& getlist() const { return m_list; }

	protected:
		List m_list;
	};

} // namespace fastdelegate

#endif // !defined(FASTDELEGATELIST_H)
