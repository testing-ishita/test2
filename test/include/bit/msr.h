/*
 * msr.h
 *
 *  Created on: 17 Nov 2010
 *      Author: engineer
 */

#ifndef MSR_H_
#define MSR_H_

extern void	vReadMsr (UINT32 dIndex, UINT32* dMsb, UINT32* dLsb);
extern void	vWriteMsr (UINT32 dIndex, UINT32 dMsb, UINT32 dLsb);

#endif /* MSR_H_ */
