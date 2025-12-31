#pragma once

//declare the delegate object,user can't call set&get function!
#define DECLARE_DELEGATE(theDelegate)                                          \
		public:                                                                \
		    theDelegate    GetDelegate(){return m_pDelegate;}                  \
		protected:                                                             \
		    void           SetDelegate(theDelegate pDelegate);                 \
		private:                                                               \
		    theDelegate    m_pDelegate;                                        \
		    gisLONG           m_flagDelegate;

//only suit to map element layer frameviewport theme label...
//the container(eg.maps layout map themes) need to destory the object
//if user did't put it in the container,need to destory self(use delete)
#define DEFAULT_CONSTRUCT(theClass)                                            \
		protected:                                                             \
		    theClass();                                                        \
		    theClass(const theClass& );                                        \
		public:                                                                \
		    virtual ~theClass();                                               \
		    static 	theClass* CreateInstance(){return new theClass();}	

//construct delegate
#define INIT_DELEGATE(theDelegateClass)                                        \
	    m_pDelegate = new theDelegateClass();                                  \
	    m_flagDelegate = 0;                                                    \
	    SetDelegate(m_pDelegate);

//deconstruct delegate
#define DELETE_DELEGATE                                                        \
	    if(m_flagDelegate == 0 && m_pDelegate)                                 \
	        delete m_pDelegate;                                                \
	    m_pDelegate = NULL;  

#define BEGIN_SET_DELEGATE(theOwnerClass,theDelegateClass)                     \
	    void theOwnerClass::SetDelegate(theDelegateClass pDelegate)            \
		{ 

#define SET_DELEGATE                                                           \
	    if(m_pDelegate != pDelegate)                                           \
		{                                                                      \
		    if(m_flagDelegate == 0 && m_pDelegate)                             \
		        delete m_pDelegate;                                            \
		                                                                       \
		    m_pDelegate = pDelegate;                                           \
		    m_flagDelegate = 1;                                                \
		}

#define END_SET_DELEGATE                         }

//set events delegate
#define SET_BASE_DELEGATE(theBaseOwnerClass,theBaseDelegateclass)              \
		{                                                                      \
		    theBaseDelegateclass pBaseDelegate = dynamic_cast<theBaseDelegateclass>(pDelegate);\
		                                                                       \
		    if(pBaseDelegate)                                                  \
		        theBaseOwnerClass::SetDelegate(pBaseDelegate);                 \
		}

#define INIT_OWNER                                                             \
	    m_pDelegate->SetOwner(this);

//set base owner
#define SET_BASE_OWNER(theBaseOwnerClass,theBaseDelegateclass)                 \
		{                                                                      \
		    theBaseOwnerClass* pBaseOwner = dynamic_cast<theBaseOwnerClass*>(pOwner);\
		                                                                       \
		    if(pBaseOwner)                                                     \
		        theBaseDelegateclass::SetOwner(pBaseOwner);                    \
		}


///////////////////////////////example/////////////////////////////////////////
/*
为了能够隐藏实现和支持用户扩展，请按以下示例实现
每个外层类都必须拥有SetDelegate函数
每个内层类都可以拥有SetOwner函数
SetDelegate函数内部还需要调用父类的SetDelegate函数
SetOwner函数内部还需要调用父类的SetOwner函数
只有虚方法的调用才使用pOwner指针
typedef class Ci_Render* PTRender;
class CGRender
{
public:
	CGRender();
	~CGRender();
	void                PrintMessage();                    //no virtual function
	virtual gisLONG        Render();                          //virtual function
	virtual REG_INFO*   OnGetRegInfo();                    //virtual function

	DECLARE_DELEGATE(PTRender)
};

typedef class Ci_MyRender* PTMyRender;
class CGMyRender : public CGRender
{
public:
	CGMyRender();
	~CGMyRender();
	virtual REG_INFO*   OnGetRegInfo();                    //overwrite

	DECLARE_DELEGATE(PTMyRender)
};

//////////////////////////////////////////////////////////////////////////
CGRender::CGRender()
{
	INIT_DELEGATE(Ci_Render)
	INIT_OWNER
}

CGRender::~CGRender()
{
	DELETE_DELEGATE
}

BEGIN_SET_DELEGATE(CGRender,PTRender) 
	SET_DELEGATE
	//if this class has parent,should set parent's delegate!!!
END_SET_DELEGATE

gisLONG CGRender::Render()
{
	return m_pDelegate->Render();
}

REG_INFO* CGRender::OnGetRegInfo()
{
	return m_pDelegate->OnGetRegInfo();
}

//////////////////////////////////////////////////////////////////////////
CGMyRender::CGMyRender()
{
	INIT_DELEGATE(Ci_MyRender)
	INIT_OWNER
}

CGMyRender::~CGMyRender()
{
	DELETE_DELEGATE
}

BEGIN_SET_DELEGATE(CGMyRender,PTMyRender) 
	SET_DELEGATE
	//if this class has parent,should set parent's delegate!!!
	//because this class has parent,so i should set parent's delegate!
	SET_BASE_DELEGATE(CGRender,PTRender)
END_SET_DELEGATE

REG_INFO* CGMyRender::OnGetRegInfo()
{
	return m_pDelegate->OnGetRegInfo();
}

//////////////////////////////////////////////////////////////////////////
class Ci_Render
{
public:
	Ci_Render();
	~Ci_Render();
	void                SetOwner(CGRender* pOwner);        //must!!!add SetOwner function!
	void                PrintMessage();                    //no virtual function
	virtual gisLONG        Render();                          //virtual function
	virtual REG_INFO*   OnGetRegInfo();                    //virtual function
private:
	CGRender*           m_pOwner;
};

class Ci_MyRender : public Ci_Render                       //must inherit!!!
{
public:
	Ci_MyRender();
	~Ci_MyRender();
	void                SetOwner(CGMyRender* pOwner);      //must!!!add SetOwner function!
	virtual REG_INFO*   OnGetRegInfo();                    //overwrite
private:
	CGMyRender*         m_pOwner;
};


//////////////////////////////////////////////////////////////////////////
Ci_Render::Ci_Render()
{
	m_pOwner = NULL;
}

Ci_Render::~Ci_Render()
{
}

void Ci_Render::SetOwner(CGRender* pOwner)
{
	m_pOwner = pOwner;
}

gisLONG Ci_Render::Render()
{
	//notice below logic!!!

	//REG_INFO*      pRegInfo = OnGetRegInfo();     //is error!!!

	//should is,our purpose is call the out virtual member function!!!
	//if OnGetRegInfo is virtual member function,we must use m_pOwner to call!
	//if the called member function is not virtual function,not require m_pOwner!!!
	//if m_pOwner is point to CGMyRender obj,will call its OnGetRegInfo function! 
	PrintMessage();

	REG_INFO*           pRegInfo = m_pOwner->OnGetRegInfo();

	return 1;
}

REG_INFO* Ci_Render::OnGetRegInfo()
{
	//first implementation

	return pRegInfo;
}

//////////////////////////////////////////////////////////////////////////
Ci_MyRender::Ci_MyRender()
{
	m_pOwner = NULL;
}

Ci_MyRender::~Ci_MyRender()
{

}

void Ci_MyRender::SetOwner(CGMyRender* pOwner)
{
	m_pOwner = pOwner;

	//if this class has parent,should set parent's owner!!!
	//because this class has parent,so i should set parent's owner!
	SET_BASE_OWNER(CGRender,Ci_Render)
}

REG_INFO* Ci_MyRender::OnGetRegInfo()
{
	//second implementation

	return pRegInfo;
}
*/