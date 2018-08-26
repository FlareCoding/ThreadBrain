# ThreadBrain
Single header library to help with scheduling, running, and controlling tasks in a separate thread

**Usage:**

`TB_Thread* threadHandle; // handle to the thread `  
`TB::StartTask(TaskToExecute, false, &handle); // Starts execution of the task in another thread`  
Second parameter of StartTask function tells whether or not task should be repetitively  
executed in a loop. If it is false, then the task is executed only once.  

**If you start executing the task in a loop:**  
If the second parameter of StartTask is true, then the task will be executed repetitively  
in a while loop. To control the thread's activity you can send "TB_Notification"s to the thread:  
`handle->Notify(TB_PAUSE_NOTIFY); // pauses thread activity until resumed `  
`handle->Notify(TB_RESUME_NOTIFY); // resumes the thread`  
`handle->Notify(TB_SLEEP_NOTIFY); // pauses the thread for a set period of time`  
`handle->Notify(TB_CLOSE_NOTIFY); // tell the thread to break out of the loop and stop execution of the task`  
