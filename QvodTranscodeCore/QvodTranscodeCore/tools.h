#pragma once

class CTaskIDManager
{
public:
	static unsigned int GetTaskID(){return m_nTaskId++;}
protected:
	static unsigned int m_nTaskId;
};