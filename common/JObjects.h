#ifndef _JOBJECTS_
#define _JOBJECTS_

#pragma once
//#pragma warning(disable : 4786 ) // identifier debug info is too long
// !!!!!!!!!!!!!!!!!!!!! Too long symbol name warning will be disabled in entire project

#if _MSC_VER < 1300
#include <comdef.h>
#include "afxtempl.h"
#else
#include <comutil.h>
#endif

#ifndef _VERIFY
#ifdef _DEBUG
#define _VERIFY(expr) _ASSERT(expr)
#else
#define _VERIFY(expr) (expr)
#endif
#endif

#define JPTR_ALLOW_PTR_CAST
#undef JPTR_CLEAR_METHOD

// Helpers ============================================================================================

#define JPROPERTY_R_R(type, pname) public: \
		__declspec(property(get=get##pname)) type pname; \
	protected: \
		type m_##pname;

#define JPROPERTY_RW_W(type, pname) public: \
		__declspec(property(get=get##pname,put=set##pname)) type pname; \
		type get##pname() const { return m_##pname; } \
	protected: \
		type m_##pname;

#define JPROPERTY_R(type, pname) public: \
		__declspec(property(get=get##pname)) type pname; \
		type get##pname() const { return m_##pname; } \
	protected: \
		type m_##pname;

#define JPROPERTY_RREF(type, pname) public: \
		__declspec(property(get=get##pname)) type pname; \
		type& get##pname() { return m_##pname; } \
	protected: \
		type m_##pname;

#define JPROPERTY_R_CONST(type, pname) public: \
		__declspec(property(get=get##pname)) const type pname; \
		const type get##pname() const { return m_##pname; } \
	protected: \
		type m_##pname;

#define JPROPERTY_RREF_CONST(type, pname) public: \
		__declspec(property(get=get##pname)) const type pname; \
		const type& get##pname() const { return m_##pname; } \
	protected: \
		type m_##pname;

#define JPROPERTY_RREF_CONST_R(type, pname) public: \
		__declspec(property(get=get##pname)) const type pname; \
	protected: \
		type m_##pname;

#define JPROPERTY_RW(type, pname) public: \
		__declspec(property(get=get##pname,put=set##pname)) type pname; \
		type get##pname() const { return m_##pname; } \
		void set##pname(const type& val) { m_##pname = val; } \
	protected: \
		type m_##pname;

// thread safe JClasses =======================================================================================

template <class T> class JPtr;

class __declspec(uuid("{13826F21-32D8-4ff0-A7A9-C280BC821A4F}")) JClass {
	friend class JPtr<JClass>;
private:
#ifndef JPTR_ALLOW_PTR_CAST
	JClass* operator &() { return this; }
#endif
protected:
	mutable volatile LONG m_RefCount;

#ifdef JPTR_ALLOW_PTR_CAST
public:
#endif
	JClass() : m_RefCount(0) {
#ifdef _DEBUG
		InterlockedIncrement(&NumJObjects);
#endif
	}
	JClass(const JClass&) : m_RefCount(0) {
#ifdef _DEBUG
		InterlockedIncrement(&NumJObjects);
#endif
	}
	virtual ~JClass() { 
		//_ASSERT(m_RefCount == 0);
#ifdef _DEBUG
		InterlockedDecrement(&NumJObjects);
#endif
	}

	virtual void beforeDestruct() {
	}

	JClass& __fastcall operator=(const JClass&) {
		return *this;
	}

	LONG __fastcall JAddRef() const throw() {
		return InterlockedIncrement(&m_RefCount);
	}
	LONG __fastcall JRelease() const throw() {
		LONG rc = InterlockedDecrement(&m_RefCount);
		if(rc == 0) {
			((JClass*)(this))->beforeDestruct();
			delete this;
		}
		return rc;
	}
	LONG __fastcall JGetRefCount() const { return m_RefCount; }
public:
#ifdef _DEBUG
	static volatile LONG NumJObjects;
#endif
};

class JComClass : public JClass, public IUnknown {
public:
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void **ppvObject)
	{
		if(iid == __uuidof(IUnknown)) {
			*ppvObject = (IUnknown*)this;
			JAddRef();
			return S_OK;
		} else if(iid == __uuidof(JClass)) {
			_ASSERT(ppvObject == NULL);
			return S_OK;
		} else
			return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef() {
		return JAddRef();
	}

	ULONG STDMETHODCALLTYPE Release() {
		return JRelease();
	}
};

#define JCOMCLASS_QUERY(classname, classparent)

#pragma optimize("a", on)

// typedef enum JPTR_CAST { JPTR_DYNAMIC_CAST };

// class JPTR_DUMMY_NOT_AVAIL;

template <class T> class JPtr
{
protected:
	T* objectRef;

public:
	JPtr() throw() : objectRef(NULL) {}
	JPtr(int null)
		: objectRef(NULL)
	{
		_ASSERT(null == 0);
	}
	JPtr(T* init) : objectRef(init) {
		if(objectRef)
			objectRef->JAddRef();
	}
	JPtr(IUnknown* obj)
	{
		if(obj) {
			HRESULT hr = obj->QueryInterface(__uuidof(JClass), NULL);
			if(FAILED(hr)) {
				objectRef = NULL;
			} else {
				JClass* jc = (JClass*)obj;
				objectRef = dynamic_cast<T*>(jc);
				if(objectRef)
					objectRef->JAddRef();
			}
		} else {
			objectRef = NULL;
		}
	}
	JPtr(const _variant_t& vt)
	{
		IUnknown* obj = vt;
		if(obj) {
			HRESULT hr = obj->QueryInterface(__uuidof(JClass), NULL);
			if(FAILED(hr)) {
				objectRef = NULL;
			} else {
				JClass* jc = (JClass*)obj;
				objectRef = dynamic_cast<T*>(jc);
				if(objectRef)
					objectRef->JAddRef();
			}
		} else {
			objectRef = NULL;
		}
	}
	template<class TS> JPtr(const JPtr<TS>& init) : objectRef((TS*)(((JPtr*)&init)->objectRef)) {
		if(objectRef)
			objectRef->JAddRef();
	}
	JPtr(const JPtr& init) : objectRef(init.objectRef) {
		if(objectRef)
			objectRef->JAddRef();
	}
	~JPtr() {
		if(objectRef)
			objectRef->JRelease();
	}

	JPtr& __fastcall operator=(int null)
	{
		_ASSERT(null == 0);

		if(objectRef) {
			objectRef->JRelease();
			objectRef = NULL;
		}

		return *this;
	}
	template<class TS> JPtr& __fastcall operator=(TS* right) {
		T* prev = objectRef;
		objectRef = right;
		if(objectRef)
			objectRef->JAddRef();
		if(prev)
			prev->JRelease();
		return *this;
	}
	template<class TS> JPtr& __fastcall operator=(const JPtr<TS>& right) {
		T* prev = objectRef;
		objectRef = (TS*)(((JPtr*)&right)->objectRef);
		if(objectRef)
			objectRef->JAddRef();
		if(prev)
			prev->JRelease();
		return *this;
	}
	JPtr& __fastcall operator=(const JPtr& right) { // some error? MSVC don't use template for equal type assigments
		T* prev = objectRef;
		objectRef = right.objectRef;
		if(objectRef)
			objectRef->JAddRef();
		if(prev)
			prev->JRelease();
		return *this;
	}

/*	template<class TS> bool __fastcall operator ==(const TS* obj) const { return ((const TS*)objectRef == obj); }
	template<class TS> bool __fastcall operator !=(const TS* obj) const { return ((const TS*)objectRef != obj); }

	template<class TS> bool __fastcall operator ==(const JPtr<TS>& obj) const {
		return (objectRef == ((JPtr*)&obj)->objectRef); }
	template<class TS> bool __fastcall operator !=(const JPtr<TS>& obj) const {
		return (objectRef != ((JPtr*)&obj)->objectRef); }
	template<class TS> bool __fastcall operator <(const JPtr<TS>& obj) const {
		return (objectRef < ((JPtr*)&obj)->objectRef); }
	template<class TS> bool __fastcall operator <=(const JPtr<TS>& obj) const {
		return (objectRef <= ((JPtr*)&obj)->objectRef); }
	template<class TS> bool __fastcall operator >(const JPtr<TS>& obj) const {
		return (objectRef > ((JPtr*)&obj)->objectRef); }
	template<class TS> bool __fastcall operator >=(const JPtr<TS>& obj) const {
		return (objectRef >= ((JPtr*)&obj)->objectRef); }*/

#ifdef JPTR_CLEAR_METHOD
	void __fastcall Clear() {
		if(objectRef)
			objectRef->JRelease();
		objectRef = NULL;
	}
#endif

	T* __fastcall operator->() const { return (T*)objectRef; }

#ifdef JPTR_ALLOW_PTR_CAST
	__fastcall operator T*() const { return objectRef; }
	__fastcall operator variant_t() const { const IUnknown* i = objectRef; return (IUnknown*)i; }
#else
	T& __fastcall operator*() const { return *objectRef; }

	unsigned int __fastcall GetKey() const { // KEY - ONLY FOR COMPARE PURPOSES!
		return ~((unsigned int)objectRef);
	}
	__fastcall operator bool() const {
		return (objectRef != NULL);
	}
#endif

	/*
	template<class TS> JPtr(const JPtr<TS>& src, JPTR_CAST) : objectRef((jdynamic_cast<T>(src)).objectRef) {
		if(objectRef)
			objectRef->JAddRef();
	}
	*/

//	template<class TD> friend inline JPtr<TD> __fastcall jdynamic_cast(const JPtr& src);
//	JPtr* operator &() { ASSERT_MSG(0, "Trying to call operator& for JPtr"); return NULL; }
};

template<class TD, class TS> inline JPtr<TD> __fastcall jdynamic_cast(const JPtr<TS>& from) 
{
//#pragma message("jdynamic_cast temporal hack (friend template is not work)")
	//return NULL;
	class JPtr_Int {
	public:
		TS* objectRef;
	};
	return JPtr<TD>(dynamic_cast<TD*>(((JPtr_Int*)&from)->objectRef));
//	return JPtr<TD>(dynamic_cast<TD*>(from.objectRef));
}

template<class TD, class TS> inline JPtr<TD> __fastcall jstatic_cast(const JPtr<TS>& from) 
{
	class JPtr_Int {
	public:
		TS* objectRef;
	};
	return JPtr<TD>(static_cast<TD*>(((JPtr_Int*)&from)->objectRef));
}

// MFC - to JClasses ports =============================================================================

template <class T> class JObject : public JClass, public T {
	friend class JPtr<JObject<T>>;
protected:
	JObject() {} // JClasses must be instantiated through Create only
	JObject(T& init) : T(init) {}

public:
	static JPtr<JObject<T>> Create(T& init) { return JPtr<JObject<T>>(new JObject<T>(init)); }
};

#if _MSC_VER < 1300
template <class TYPE, class ARG_TYPE> class JList : public JClass, public CList<TYPE, ARG_TYPE> {
	friend class JPtr<JList<TYPE, ARG_TYPE> >;
protected:
	JList(int nBlockSize) : JClass(), CList<TYPE, ARG_TYPE>(nBlockSize) {}

public:
	static JPtr<JList> Create(int nBlockSize = 10) { return JPtr<JList>(new JList(nBlockSize)); }
};
#endif

// JClass-based arrays container =============================================================================

template<typename T>
class JArrayRef : public JClass
{
protected:
	T* arrayRef;

public:
	typedef T element_type;

	explicit JArrayRef(T* init = 0) throw() : arrayRef(init) {}
	JArrayRef(size_t num) {
		arrayRef = new T[num];
	}
	~JArrayRef() {
		delete [] arrayRef;
		arrayRef = 0;
	}

	T* __fastcall operator=(T* right) {
		if (right != arrayRef) {
			delete [] arrayRef;
			arrayRef = right;
		}
	}
	T* __fastcall operator->() const {
		_ASSERT(arrayRef);
		return arrayRef;
	}
#ifdef JPTR_ALLOW_PTR_CAST
	__fastcall operator T*() const {
		return arrayRef;
	}
#else
	T& __fastcall operator*() const {
		_ASSERT(arrayRef);
		return *arrayRef;
	}

	unsigned int __fastcall GetKey() const { // KEY - ONLY FOR COMPARE PURPOSES!
		return ~((unsigned int)arrayRef);
	}
	__fastcall operator bool() const {
		return (arrayRef != NULL);
	}
#endif
	T& __fastcall operator[](int elem) const {
		_ASSERT(arrayRef);
		return arrayRef[elem];
	}

	T* __fastcall get() const throw() {
		return arrayRef;
	}
	T* __fastcall release() throw()
	{
		T* tmp = arrayRef;
		arrayRef = 0;
		return tmp;
	}
	void __fastcall reset(T* ptr = 0)
	{
		if (ptr != arrayRef)
		{
			delete [] arrayRef;
			arrayRef = ptr;
		}
	}

private:
	// Disable those operations
	JArrayRef(const JArrayRef& init) : arrayRef(0)
	{
		_ASSERT(false);
	}
	JArrayRef& __fastcall operator=(const JArrayRef& right)
	{
		if (&right != this)
		{
			_ASSERT(false);
		}
		return *this;
	}
};

// JIDClass =======================================================================================

typedef int JID;
typedef std::set<JID> SetJID;

template<class T, typename Tkey = JID> class CIDClass
{
public:
	typedef Tkey id_type;
	typedef std::map< id_type, JPtr<T> > IDS; // not hash map to preserve order across network and serialization

protected:
	static IDS s_IDs;
	JPROPERTY_R( const id_type, ID );

public:
	CIDClass( id_type id ) : m_ID(id)
	{
		_VERIFY( s_IDs.insert( IDS::value_type( id, (T*)this ) ).second );
	}

	static const IDS& getIDs()
	{
		return s_IDs;
	}

	static JPtr<T> get( id_type id )
	{
		IDS::const_iterator i = s_IDs.find(id);
		return i != s_IDs.end() ? i->second : 0;
	}

	static void destroy( id_type id )
	{
		s_IDs.erase(id);
	}

	static void destroySet( const std::set<id_type>& set )
	{
		for (std::set<id_type>::const_iterator iter = set.begin(); iter < set.end(); iter++) {
			s_IDs.erase(*iter);
		}
	}

	static void destroyAll()
	{
		s_IDs.clear();
	}
};

template<class T, typename Tkey> typename CIDClass<T, Tkey>::IDS CIDClass<T, Tkey>::s_IDs;

template<class T, typename Tkey = JID> class JIDClass : public JClass, public CIDClass<T, Tkey>
{
public:
	JIDClass( Tkey id ) : CIDClass(id) {}
};

#pragma optimize("", on)

#endif // _JOBJECTS_
