#pragma once

// Encapsulate the resources released by the application, you can put different types of resources in the same queue

class StaleResourceWrapper final
{
public:
	template <typename ResourceType >
	static StaleResourceWrapper Create(ResourceType&& resource)
	{
		// Class template
		class SpecificStaleResource final : public StaleResourceBase
		{
		public:
			SpecificStaleResource(ResourceType resource) :
				m_SpecificResource(std::move(resource))
			{}

			SpecificStaleResource(const SpecificStaleResource&) = delete;
			SpecificStaleResource(SpecificStaleResource&&) = delete;
			SpecificStaleResource& operator= (const SpecificStaleResource&) = delete;
			SpecificStaleResource& operator= (SpecificStaleResource&&) = delete;

			// delete this
			virtual void Release() override	final
			{
				delete this;
			}

		private:
			ResourceType m_SpecificResource;
		};

		return StaleResourceWrapper(new SpecificStaleResource(resource));
	}

	StaleResourceWrapper(StaleResourceWrapper&& rhs) noexcept :
		m_StaleResource(rhs.m_StaleResource)
	{
		rhs.m_StaleResource = nullptr;
	}

	StaleResourceWrapper(const StaleResourceWrapper& rhs) = delete;
	StaleResourceWrapper& operator= (const StaleResourceWrapper&) = delete;
	StaleResourceWrapper& operator= (const StaleResourceWrapper&&) = delete;

	~StaleResourceWrapper()
	{
		if (m_StaleResource != nullptr)
			m_StaleResource->Release();
	}

	void GiveUpOwnership()
	{
		m_StaleResource = nullptr;
	}

private:
	// The base class of the released resource, the Wrapper object holds a base class pointer
	class StaleResourceBase
	{
	public:
		virtual void Release() = 0;
	};

	// Private constructor, Wrapper object can only be created through Create
	StaleResourceWrapper(StaleResourceBase* staleResource) :
		m_StaleResource(staleResource)
	{

	}

	StaleResourceBase* m_StaleResource;
};
