/*
 * sound.h
 *
 * Created on: Sep 16, 2015
 *     Author: Ekawahyu Susilo
 *
 * Copyright (c) 2015, Chongqing Aisenke Electronic Technology Co., Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of the copyright holder.
 *
 */

#ifndef SOUND_H_
#define SOUND_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SOUND_SECOND    RTIMER_SECOND

#define C4  (500000/261)
#define D4  (500000/293)
#define E4  (500000/329)
#define F4  (500000/349)
#define G4  (500000/392)
#define A4  (500000/440)
#define B4  (500000/493)
#define C5  (500000/523)
#define D5  (500000/587)
#define E5  (500000/659)
#define F5  (500000/698)
#define G5  (500000/783)
#define A5  (500000/880)
#define B5  (500000/987)
#define C6  (500000/1046)
#define D6  (500000/1174)
#define E6  (500000/1318)
#define F6  (500000/1396)
#define G6  (500000/1567)
#define A6  (500000/1760)
#define B6  (500000/1975)
#define C7  (500000/2093)
#define D7  (500000/2349)
#define E7  (500000/2637)
#define F7  (500000/2793)
#define G7  (500000/3135)
#define A7  (500000/3520)
#define B7  (500000/3951)
#define C8  (500000/4186)

void sound_init(void);
void sound_play(uint16_t note, uint16_t duration);

#ifdef __cplusplus
}
#endif

#endif /* SOUND_H_ */
