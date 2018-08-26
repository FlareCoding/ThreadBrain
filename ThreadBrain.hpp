/*
*
*	Created on 25th of August, 2018
*	Copyright (c) by Albert Slepak. All rights reserved.
*
*/

#pragma once
#include <vector>
#include <thread>
#include <Windows.h>


// Prototype for a task that will be executed in an infinite loop
typedef void(*TB_Task)(void);

// Notification types to be sent to a thread
enum TB_Notification
{
	/// <summary> Empty notification that will do nothing, it can act as a placeholder </summary>
	TB_EMPTY_NOTIFICATION,

	/// <summary>  Notifies the thread to exit out of the infite loop and exit the thread </summary>
	TB_CLOSE_NOTIFY,

	/// <summary> Notifies the thread to pause and not execute any tasks until TB_RESUME_NOTIFY is recieved </summary>
	TB_PAUSE_NOTIFY,

	/// <summary> Notifies the thread to resume its activity after recieving TB_PAUSE_NOTIFY notification </summary>
	TB_RESUME_NOTIFY,

	/// <summary> Notifies the thread to sleep for a period that can be set with SetSleepPeriod() </summary>
	TB_SLEEP_NOTIFY,

	/// <summary> Manually execute the task function </summary>
	TB_FORCE_TASK_EXECUTE_NOTIFY
};

class TB_Thread
{
public:
	TB_Thread(TB_Task task)
	{
		this->task = task; // setting the function that will be executed
	}

	// Executed the task once
	void __stdcall SingleExecute()
	{
		// wrapping this into a lambda because of intitialization issues with std::thread's constructor
		running_thread = std::thread([this]
		{
			task();
		});
		running_thread.detach(); // detaching the thread
	}

	// Execution is in an infinite while loop and
	// can be controlled by sending TB_Notifications
	void __stdcall InfiniteExecute()
	{
		// Executing the while loop in a lambda function
		// because it's easier
		running_thread = std::thread([this]
		{
			while (true)
			{
				Sleep(2); // Having mercy on the CPU

				// Checking for available notifications
				if (notification != TB_EMPTY_NOTIFICATION)
				{
					if (notification == TB_CLOSE_NOTIFY)	break;						// break out of the infinite loop
					if (notification == TB_PAUSE_NOTIFY)	thread_paused = true;		// pause the thread
					if (notification == TB_RESUME_NOTIFY)	thread_paused = false;		// resume the thread
					if (notification == TB_FORCE_TASK_EXECUTE_NOTIFY) task();			// manual execution of the task function
					if (notification == TB_SLEEP_NOTIFY)	Sleep(sleep_notify_period);	// hangs the thread for a set period of time

					// Reseting the notification
					notification = TB_EMPTY_NOTIFICATION;
				}

				// If thread is supposed to be paused,
				// call Sleep function to wait
				if (thread_paused)
				{
					Sleep(timeout_duration_in_miliseconds);
				}
				else
				{
					// If everything is clear, execute the task
					task();
				}
			}
		});
		running_thread.detach(); // detaching the thread
	}

	// Sends a signal to the thread with the TB_Notification message
	void __stdcall Notify(TB_Notification notification)
	{
		this->notification = notification;
	}

	// Sets default wait period in miliseconds in case of TB_SLEEP_NOTIFY
	// 1 second (1000 milis) is set by default
	void __stdcall SetSleepPeriod(int miliseconds)
	{
		this->sleep_notify_period = miliseconds;
	}

private:
	std::thread running_thread;
	TB_Task task;
	TB_Notification notification = TB_EMPTY_NOTIFICATION;
	bool thread_paused = false;
	int timeout_duration_in_miliseconds = 40;
	int sleep_notify_period = 1000; // in case of TB_SLEEP_NOTIFY
};


// Thread_Brain (TB) is a namespace for controlling threads
namespace TB
{
	// List of all created thread pointers
	std::vector<TB_Thread*> threads;

	/// <summary> Immidiately runs a given task in another daemon thread </summary>
	/// <param name="task"> A task to be executed </param>
	/// <param name="infinite_execution"> tells whether to continuesly execute the function and 
	///									  control the flow with TB_Notification calls </param>
	/// <param name="thread_handle"> pointer to a handle to the thread </param>
	/// <param name="scheduled_delay"> Optional delay in miliseconds (0 by default) to wait before starting the thread </param>
	void __stdcall StartTask(
		TB_Task task,
		bool infinite_execution,
		TB_Thread** thread_handle,
		int scheduled_delay = 0
	)
	{
		TB_Thread* tb_thread = new TB_Thread(task); // Creating the thread object and assigning a task

		// Creating a handle to the thread
		if (thread_handle != nullptr)
		{
			*thread_handle = tb_thread;
		}

		// Asynchronously start the thread
		std::thread asynchronous_starter_thread([&infinite_execution, &tb_thread, &scheduled_delay]
		{
			if (scheduled_delay > 0)
			{
				Sleep(scheduled_delay);
			}

			if (!infinite_execution)
			{
				// Task will be executed only once
				tb_thread->SingleExecute();
			}
			else
			{
				// Task will be executed infinitely
				// It will be able to be controlled through TB_Notification
				tb_thread->InfiniteExecute();
			}
		});
		asynchronous_starter_thread.detach();

		// Storing the pointer in a vector to delete at shutdown
		threads.push_back(tb_thread);
	}

	// Deletes all thread pointers and frees resources
	void __stdcall Shutdown()
	{
		// Iterates over saved thread-pointers
		for (TB_Thread* thread : threads)
		{
			delete thread; // deletes each thread pointer
		}
	}
}
