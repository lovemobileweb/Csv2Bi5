
#include "LZMA/easylzma/compress.h"  

#include <string.h>  
#include <assert.h>  
#include <iostream>
#include <fstream>
#include "Log.h"

using namespace std;

struct dataStream
{
	const unsigned char * inData;
	size_t inLen;

	unsigned char * outData;
	size_t outLen;
};

/* an input callback that will be passed to elzma_compress_run(),
* it reads from a memory buffer */
static int
inputCallback(void *ctx, void *buf, size_t * size)
{
	size_t rd = 0;
	struct dataStream * ds = (struct dataStream *) ctx;
	assert(ds != NULL);

	rd = (ds->inLen < *size) ? ds->inLen : *size;

	if (rd > 0) {
		memcpy(buf, (void *)ds->inData, rd);
		ds->inData += rd;
		ds->inLen -= rd;
	}

	*size = rd;

	return 0;
}

/* an ouput callback that will be passed to elzma_compress_run(),
* it reallocs and writes to a memory buffer */
static size_t
outputCallback(void *ctx, const void *buf, size_t size)
{
	struct dataStream * ds = (struct dataStream *) ctx;
	assert(ds != NULL);

	if (size > 0) {
		ds->outData = (unsigned char *)realloc(ds->outData, ds->outLen + size);
		memcpy((void *)(ds->outData + ds->outLen), buf, size);
		ds->outLen += size;
	}

	return size;
}

/* a function that will compress data using a 1mb dictionary and a
* client specified encoding format (one of ELZMA_lzip or ELZMA_lzma) */
int
simpleCompress(elzma_file_format format, const char * inPath, const char * outPath)
{
	size_t inLen;
	unsigned char * inData;
	unsigned char * outData;
	size_t outLen;
	int rc;
	elzma_compress_handle hand;

	/* allocate compression handle */
	hand = elzma_compress_alloc();
	assert(hand != NULL);

	/* read inData from inPath file */
	ifstream in(inPath, ios::in | ios::binary | ios::ate);
	if (!in.is_open())
	{
		elzma_compress_free(&hand);
		Log::WriteLine("Failed to open BI5 tmp file %s", inPath);
		return ELZMA_E_BAD_PARAMS;
	}
	inLen = in.tellg();
	inData = (unsigned char *)malloc(inLen);
	in.seekg(0);
	in.read((char *)inData, inLen);
	if ((size_t)in.tellg() != inLen)
	{
		elzma_compress_free(&hand);
		Log::WriteLine("Failed to read %d bytes from BI5 tmp file %s(%d)", inLen, inPath, in.tellg());
		if (inData != NULL) free(inData);
		return ELZMA_E_INPUT_ERROR;
	}
	in.close();
	
	/* configure the compression run with mostly default parameters  */
	rc = elzma_compress_config(hand, ELZMA_LC_DEFAULT,
		ELZMA_LP_DEFAULT, ELZMA_PB_DEFAULT,
		5, (1 << 20) /* 1mb */,
		format, inLen);

	/* fail if we couldn't allocate */
	if (rc != ELZMA_E_OK) {
		elzma_compress_free(&hand);
		Log::WriteLine("Failed to allocate memory for BI5 tmp file %s", inPath);
		if (inData != NULL) free(inData);
		return rc;
	}

	/* now run the compression */
	{
		/* set up the context structure that will be passed to
		* stream callbacks */
		struct dataStream ds;
		ds.inData = inData;
		ds.inLen = inLen;
		ds.outData = NULL;
		ds.outLen = 0;

		/* run the streaming compression */
		rc = elzma_compress_run(hand, inputCallback, (void *)&ds,
			outputCallback, (void *)&ds, NULL, NULL);

		if (rc != ELZMA_E_OK) {
			if (inData != NULL) free(inData);
			if (ds.outData != NULL) free(ds.outData);
			elzma_compress_free(&hand);
			return rc;
		}

		outData = ds.outData;
		outLen = ds.outLen;
	}

	if (outData != NULL)
	{
		ofstream out(outPath, ios::out | ios::binary | ios::trunc);
		if (!out.is_open())
		{
			elzma_compress_free(&hand);
			Log::WriteLine("Failed to create BI5 file %s", outPath);
			if (outData != NULL) free(outData);
			if (inData != NULL) free(inData);
			return ELZMA_E_BAD_PARAMS;
		}
		out.write((char *)outData, outLen);
		if ((size_t)out.tellp() != outLen)
		{
			elzma_compress_free(&hand);
			Log::WriteLine("Failed to write %d bytes from BI5 file %s(%d)", outLen, outPath, out.tellp());
			if (outData != NULL) free(outData);
			if (inData != NULL) free(inData);
			return ELZMA_E_INPUT_ERROR;
		}
		out.close();
		if (outData != NULL) free(outData);
	}
	else
		Log::WriteLine("Failed to compress BI5 tmp file %s", inPath);
	if (inData != NULL) free(inData);
	if (std::remove(inPath) != 0)
		Log::WriteLine("Failed to remove BI5 tmp file %s", inPath);

	elzma_compress_free(&hand);

	return rc;
}
