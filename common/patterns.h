#pragma once

// Static constructor/destructor pattern
template<class T>
class initdoneable {
	struct watchdog {
		struct watchdog_helper : T {friend struct watchdog;};
		watchdog() {watchdog_helper::initclass();}
		~watchdog() {watchdog_helper::doneclass();}
	};
protected:
	// default versions
	static void initclass() {}
	static void doneclass() {}
public: // give opportunity to initialize before first T-constructor call
	// force instantiating the watchdog variable
	initdoneable() {static const watchdog wd_;}
};

__interface __declspec(uuid("{44609DB6-CEE7-4584-954E-4C7C3C390220}"))
IRunnable
{
	int Run();
};

__interface __declspec(uuid("{C3754A84-1800-4975-BCD2-8C4B943F95B2}"))
IApplication : public IRunnable
{
	void Init();
	void Done();
};

__interface __declspec(uuid("{1E7027B0-5EF6-4747-86CC-0880E218EE27}"))
IService : public IApplication
{
	enum EState {eNone, eInit, eDone, eWantRun, eRunning, eWantStop, eStopped, eSuspended};

	EState getState() const;
	void Stop();
	void Suspend();
	void Resume();
};