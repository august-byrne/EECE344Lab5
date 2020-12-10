/*
 * AlarmWave.h the the header file for AlarmWave.c, and it makes AlarmWaveInit()
 * and AlarmWaveSetMode() accessible to other files if they include AlarmWave.h
 *
 *  Created on: November 18, 2020
 *      Author: August Byrne
 */

#ifndef ALARMWAVE_H_
#define ALARMWAVE_H_

void AlarmWaveInit(void);
void AlarmWaveControlTask(void);
void AlarmWaveSetMode(void);

#endif /* ALARMWAVE_H_ */
