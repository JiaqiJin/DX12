#pragma once

template<typename T>
class Singleton
{
public:
	Singleton()
	{
		assert(!m_Singleton);
		m_Singleton = static_cast<T*>(this);
	}

	~Singleton()
	{
		assert(m_Singleton);
		m_Singleton = nullptr;
	}
	
	Singleton(const Singleton<T>&) = delete;
	Singleton& operator=(const Singleton<T>&) = delete;

	static T& GetSingleton()
	{
		return *m_Singleton;
	}

	static T* GetSingletonPtr()
	{
		return m_Singleton;
	}

protected:
	static T* m_Singleton;
};

template<typename T>
T* Singleton<T>::m_Singleton = nullptr;