#include <ZLFP10Controller.h>

#include "DewPoint.h"



float DewPoint(float celsius, float humidity)
{
	float a = 17.271;
	float b = 237.7;
	float temp = (a * celsius) / (b + celsius) + log(humidity / 100);
	float Td = (b * temp) / (a - temp);
	return Td;

	return 0;
}
float FtoC(float Fin)
{
	float retval;
	retval = (Fin - 32) * 5 / 8;
	return retval;
};
float CtoF(float Cin)
{
	float retval;
	retval = Cin * 9 / 5 + 32;
	return retval;
}
;
