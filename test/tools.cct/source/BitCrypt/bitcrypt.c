/************************************************************************
 *                                                                      *
 *      Copyright 2010 Concurrent Technologies, all rights reserved.    *
 *                                                                      *
 *      The program below is supplied by Concurrent Technologies        *
 *      on the understanding that no responsibility is assumed by       *
 *      Concurrent Technologies for any errors contained therein.       *
 *      Furthermore, Concurrent Technologies makes no commitment to     *
 *      update or keep current the program code, and reserves the       *
 *      right to change its specifications at any time.                 *
 *                                                                      *
 *      Concurrent Technologies assumes no responsibility either for    *
 *      the use of this code or for any infringements of the patent     *
 *      or other rights of third parties which may result from its use  *
 *                                                                      *
 ************************************************************************/

/*
 * bitcrypt.c
 */

/* Version control:
 *
 * $Header: /home/cvs/cvsroot/CCT_BIT_2/tools.cct/source/BitCrypt/bitcrypt.c,v 1.2 2013-11-25 11:35:17 mgostling Exp $
 * $Log: bitcrypt.c,v $
 * Revision 1.2  2013-11-25 11:35:17  mgostling
 * Updated the version number. Fixed argument checks so help text gets displayed.
 * Made bit-shift routine use a long value for mask; when using a 16-bit compiler the shift routine was only using the lower 16-bits of the shift bitfield and zero for the upper 16 bits.
 * The same condition existed in UNIPROG, resulting in symetric encryption and decryption that hid the bug.
 * When UNIPROG migrated to a 32-bit compiler the full 32-bit value was used for the bitfield resulting in a mismatch between encryption and decryption.
 * Migrating this tool to the Watcom 32-bit compiler also fixes the problem.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <ctype.h>

#define PKT_LENGTH		5
#define ROT_LENGTH		31			// must be < 32

typedef unsigned char	UINT8;
typedef unsigned int	UINT16;
typedef unsigned long	UINT32;

const char		version[] = "V1.02";

const UINT8		dflt_xor_key[PKT_LENGTH] = { 0xA9, 0x34, 0xFB, 0x11, 0x7A };

const UINT32	dflt_rot_key = 0xF3B9D884L;


int main
(
	int		argc,
	char*	argv[]
)
{
	FILE*	psIn;
	FILE*	psOut;
	FILE*	psKey;
	UINT32	rot_key;
	UINT16	xor_sum;
	UINT8	xor_key[PKT_LENGTH];
	UINT8	buf[PKT_LENGTH];
	int		op;
	int		cnt;
	int		stat;
	int		bit;


	// Sign on

	printf ("CCT BIT-Image Copy Protection Utility - %s\n\n", version);


	// Initialise keys

	rot_key = dflt_rot_key;

	for (cnt = 0; cnt < PKT_LENGTH; cnt++)
		xor_key[cnt] = dflt_xor_key[cnt];

	
	// Check arg count
	
	if ((argc < 4) || (argc > 5))
	{
		fprintf (stderr, "Usage: bitcrypt <op> <infile> <outfile> [<key>]\n\n");
		
		fprintf (stderr, "       <op>      = 1 for encrypt, 2 for decrypt\n");
		fprintf (stderr, "       <infile>  = data for encryption or decryption\n");
		fprintf (stderr, "       <outfile> = encrypted or decrypted data\n\n");

		fprintf (stderr, "       <key>     = during encryption, file holding alternative key\n\n");

		return (1);
	}

	// Check for and read optional key

	if (argc == 5)
	{
		psKey = fopen (argv[4], "rb");

		if (psKey == NULL)
		{
			fprintf (stderr, "Cannot open key file [%s].\n\n", argv[4]);
			return (10);
		}

		stat = fread (&rot_key, sizeof (rot_key), 1, psKey);
		stat += fread (&xor_key[0], sizeof (xor_key[0]), PKT_LENGTH, psKey);

		fclose (psKey);

		if (stat != (PKT_LENGTH + 1))
		{
			fprintf (stderr, "Error reading key file.\n\n");
			return (11);
		}

		// Check keys

		xor_sum = 0;

		for (cnt = 0; cnt < PKT_LENGTH; cnt++)
			xor_sum += xor_key[cnt];

		if ((xor_sum == 0) || (rot_key == 0) || (xor_sum == (5 * 0xFF)) || (rot_key == 0xFFFFFFFF)) 
		{
			fprintf (stderr, "Bad key choice.\n\n");
			return (12);
		}
	}

	// Check <op> code

	op = 0;

	sscanf (argv[1], "%i", &op);

	if ((op < 1) || (op > 2))
	{
		fprintf (stderr, "Invalid <op>, type 'bitcrypt' for help.\n\n");
		return (20);
	}

	// Check input file

	psIn = fopen (argv[2], "rb");

	if (psIn == NULL)
	{
		fprintf (stderr, "Cannot open input file [%s].\n\n", argv[2]);
		return (21);
	}

	// Create output file

	psOut = fopen (argv[3], "wb");

	if (psOut == NULL)
	{
		fclose (psIn);

		fprintf (stderr, "Cannot create output file [%s].\n\n", argv[3]);
		return (22);
	}


	// Save or load keys

	if (op == 1)
	{
		stat = fwrite (&rot_key, sizeof (rot_key), 1, psOut);
		stat += fwrite (&xor_key[0], sizeof (xor_key[0]), PKT_LENGTH, psOut);
	}

	else	// op == 2
	{
		stat = fread (&rot_key, sizeof (rot_key), 1, psIn);
		stat += fread (&xor_key[0], sizeof (xor_key[0]), PKT_LENGTH, psIn);
	}
	
	if (stat != (PKT_LENGTH + 1))
	{
		fprintf (stderr, "Error processing key.\n\n");

		fclose (psIn);
		fclose (psOut);

		return (30);
	}


	// ==================
	// Process input file
	// ==================

	stat = 0;
	bit = 0;

	if (op == 1)	// encrypt
	{
		do {
			cnt = get_bytes (psIn, buf);

			if (cnt != 0)
			{
				xor_it (buf, xor_key);
				neg_it (buf);
				bit = rrot_it (buf, bit, rot_key);

				cnt = put_bytes (psOut, buf);

				if (cnt != 5)
					stat = 2000 + cnt;
			}

			else
			{
				if (!feof (psIn))
					stat = 1000;
			}

		} while (cnt == PKT_LENGTH);

		if (stat != 0)
			fprintf (stderr, "Error [%i] during encryption.\n\n", stat);
	}

	else
	{
		do {
			cnt = get_bytes (psIn, buf);

			if (cnt != 0)
			{
				bit = lrot_it (buf, bit, rot_key);
				neg_it (buf);
				xor_it (buf, xor_key);

				cnt = put_bytes (psOut, buf);

				if (cnt != 5)
					stat = 4000 + cnt;
			}

			else
			{
				if (!feof (psIn))
					stat = 3000;
			}

		} while (cnt == PKT_LENGTH);

		if (stat != 0)
			fprintf (stderr, "Error [%i] during decryption.\n\n", stat);		
	}

	// Clean up

	printf ("Done!");

	fclose (psIn);
	fclose (psOut);

	return (stat);

} // main ()


int get_bytes
(
	FILE*	fp,
	UINT8*	pkt
)
{
	int		i;


	// zero the buffer

	for (i = 0; i < PKT_LENGTH; i++)
		pkt[i] = 0;

	// read from file

	i = fread (pkt, sizeof (UINT8), PKT_LENGTH, fp);

	return (i);

} // get_bytes ()


int put_bytes
(
	FILE*	fp,
	UINT8*	pkt
)
{
	int		i;


	// write to file

	i = fwrite (pkt, sizeof (UINT8), PKT_LENGTH, fp);

	return (i);

} // put_bytes ()



void xor_it
(
	UINT8*	pkt,
	UINT8*	xor_key
)
{
	int		i;


	for (i = 0; i < PKT_LENGTH; i++)
		pkt[i] = pkt[i] ^ xor_key[i];
        
} // xor_it ()


void neg_it
(
	UINT8*	pkt
)
{
	int		i;


	for (i = 0; i < PKT_LENGTH; i++)
		pkt[i] = ~pkt[i];
        
} // neg_it ()


int rrot_it
(
	UINT8*  pkt,
	int		rot_bit,
	UINT32	rot_key
)
{
	int		first_bit;
	int		i;


	if (rot_key & (1 << rot_bit))
	{
		// Save LS-bit

		first_bit = pkt[0] & 0x01;

		// Rotate right (MS-bit => LS-bit) all but last byte
		
		for (i = 0; i < (PKT_LENGTH - 1); i++)
		{
			pkt[i] >>= 1;

			if ((pkt[i+1] & 0x01) != 0)
				pkt[i] |= 0x80;
		}

		// Last byte

		pkt[i] >>= 1;
		
		if (first_bit != 0)
			pkt[i] |= 0x80;
	}

	if (++rot_bit > ROT_LENGTH)
		rot_bit = 0;

	return (rot_bit);
	
} // rrot_it ()


int lrot_it
(
	UINT8*  pkt,
	int		rot_bit,
	UINT32	rot_key
)
{
	int		first_bit;
	int		i;


	if (rot_key & (1L << rot_bit))
	{
		// Save LS-bit

		first_bit = pkt[PKT_LENGTH - 1] & 0x80;

		// Rotate left (LS-bit => MS-bit) all but last byte
		
		for (i = (PKT_LENGTH - 1); i > 0 ; i--)
		{
			pkt[i] <<= 1;

			if ((pkt[i-1] & 0x80) != 0)
				pkt[i] |= 0x01;
		}

		// Last byte

		pkt[i] <<= 1;
		
		if (first_bit != 0)
			pkt[i] |= 0x01;
	}

	if (++rot_bit > ROT_LENGTH)
		rot_bit = 0;

	return (rot_bit);
	
} // lrot_it ()


