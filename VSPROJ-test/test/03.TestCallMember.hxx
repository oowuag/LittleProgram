#include <iostream>
using namespace std;

class HMI_CASParkingBrakeConfidenceJudge
{
public:
	bool& JudegParkingBrakeConfidence() 
	{
		return m_iState;
	}
private:
	bool m_iState;
};

class HMI_CarAlertServiceThread
{
public:
	bool& JudegParkingBrakeConfidence()
	{
		return m_cPBCJ.JudegParkingBrakeConfidence();
	}
	//bool JudegParkingBrakeConfidence() const
	//{
	//	return (const_cast<HMI_CASParkingBrakeConfidenceJudge &>(m_cPBCJ)).JudegParkingBrakeConfidence();
	//}
private:
	HMI_CASParkingBrakeConfidenceJudge m_cPBCJ;

};



int main()
{
	HMI_CarAlertServiceThread thread;
	thread.JudegParkingBrakeConfidence() = 0;

	return 0;
}