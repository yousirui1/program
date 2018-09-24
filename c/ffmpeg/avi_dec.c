#include <stdio.h>

#define AVIF_HASINDEX 0x00000010		//Index at end of file

typedef unsigned short BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int FOURCC;

/* MainAVIHeader */
typedef struct {
    DWORD dwMicroSecPerFrame;       //显示每桢所需的时间ns，定义avi的显示速率
    DWORD dwMaxBytesPerSec;         //最大的数据传输率
    DWORD dwPaddingGranularity;     //记录块的长度需为此值的倍数，通常是2048
    DWORD dwFlages;             //AVI文件的特殊属性，如是否包含索引块，音视频数据是否交叉存储
    DWORD dwTotalFrame;         //文件中的总桢数
    DWORD dwInitialFrames;          //说明在开始播放前需要多少桢
    DWORD dwStreams;                //文件中包含的数据流种类
    DWORD dwSuggestedBufferSize;           //建议使用的缓冲区的大小，
                           //通常为存储一桢图像以及同步声音所需要的数据之和
    DWORD dwWidth;              //图像宽
    DWORD dwHeight;             //图像高
    DWORD dwReserved[4];            //保留值
}MainAVIHeader;
typedef struct{
	WORD wLeft;
	WORD wTop;
	WORD wRight;
	WORD wBottom;
}RECT;

/* strch */
typedef struct{
	FOURCC fccType;
	
	FOURCC fccHandler;
	DWORD dwFlags;
	WORD wPriority;
	WORD wLanguage;
	DWORD dwInitalFrames;
	DWORD dwScale;
	DWORD dwRate;
	DWORD dwStart;
	DWORD dwLength;
	DWORD dwSuggestedBufferSize;
	DWORD dwQuality;
	DWORD dwSampleSize;
	RECT rcFrame;
}AVIStreamHeader;


/* strf:strh 子块是视频数据流 */
typedef struct{
	BYTE rgbBlue;
	BYTE rgbGreen;
	BYTE rgbRed;
	BYTE rgbReserved;
} RGBQUAD;

typedef struct{
	DWORD dwSize;
	DWORD dwWidth;
	DWORD dwHeight;
	WORD wPlanes;
	WORD wBitCount;
	DWORD dwCompression;
	DWORD dwSizeImage;
	DWORD dwXPelsPerMeter;
	DWORD dwYPelsPerMeter;
	DWORD dwClrUsed;
	DWORD dwClrImportant;
}BITMAPINFOHEADER;

/* strf:strh 子块是音频数据流 */
typedef struct{
	WORD wFormatTag;
	WORD wChannels;
	DWORD dwSamplesPerSec;
	DWORD dwAvgBytesPerSec;
	WORD wBlockAlign;
	WORD wBitsPerSample;
	WORD wSize;
}WAVEFORMAT;

typedef struct{
	char chunk_id[4];	
	DWORD is_key;
	DWORD pos;
	DWORD size;
}AVI_IDX;

void aviListInfo(char *pInfoBuf, int *readPos)
{
	char buf[5];
	DWORD dwValue = 0;
	int flag = 0;

	/* list */
	memset(buf, 0, 5);
	memcpy(buf, pInfoBuf + *readPos, 4);
	*readPos += 4;
	printf("list: %s\n", buf);
	
	/* list len */
	dwValue = 0;
	memcpy((char *)&dwValue, pInfoBuf + *readPos, 4);
	*readPos += 4;
	printf("list len: %d\n", dwValue);
	
	/* strl */
	memset(buf, 0, 5);
	memcpy(buf, pInfoBuf + *readPos, 4);
	*readPos += 4;
	printf("strl: %s\n", buf);
	
	/* strh */
	memset(buf, 0, 5);
	memcpy(buf, pInfoBuf + *readPos, 4);
	*readPos += 4;
	printf("strh: %s\n", buf);
	
	/* strh len */
	dwValue = 0;
	memcpy((char *)&dwValue, pInfoBuf + *readPos, 4);
	*readPos += 4;
	printf("strh len: %d\n", dwValue);

	AVIStreamHeader streamHeader;
	memset(&streamHeader, 0, sizeof(AVIStreamHeader));
	memcpy((char *)&streamHeader, pInfoBuf + *readPos, sizeof(AVIStreamHeader));
	*readPos += sizeof(AVIStreamHeader);
	{
		printf("AVIStreamHeader len: %d\n", sizeof(AVIStreamHeader));
		
		memset(buf, 0, 5);
		memcpy(buf, (char *)&streamHeader.fccType, 4);
		printf("fccType %s\n", buf);
		if(!strncmp(buf, "vids", 4))
		{
			flag = 0;
		}
		else
		{
			flag = 1;
		}
		memset(buf, 0, 5);
		memcpy(buf, (char *)&streamHeader.fccHandler, 4);
		printf(" fccHandler: %s\n", buf);
		
		printf(" Flags: %d\n", streamHeader.dwFlags);
		printf(" Priority: %d\n", streamHeader.dwQuality);
		printf(" Language: %d\n", streamHeader.wLanguage);
		printf(" InitalFrames: %d\n", streamHeader.dwInitalFrames);
		printf(" Scale: %d\n", streamHeader.dwScale);
		printf(" Rate: %d\n", streamHeader.dwRate);
		printf(" Start: %d\n", streamHeader.dwStart);
		printf(" Length: %d\n", streamHeader.dwLength);
		printf(" SuggestedBufferSize: %d\n", streamHeader.dwSuggestedBufferSize);
		printf(" Quality: %d\n", streamHeader.dwQuality);
		printf(" SampleSize: %d\n", streamHeader.dwSampleSize);
		printf(" left: %d\n", streamHeader.rcFrame.wLeft);
		printf(" top: %d\n", streamHeader.rcFrame.wTop);
		printf(" right: %d\n", streamHeader.rcFrame.wRight);
		printf(" bottom %d\n", streamHeader.rcFrame.wBottom);
	}

	/* strf */
	memset(buf, 0, 5);
	memcpy(buf, pInfoBuf + *readPos, 4);
	*readPos += 4;
	printf("strf: %s\n", buf);
	
	/* strf len */
	dwValue = 0;
	memcpy((char *)&dwValue, pInfoBuf + *readPos, 4);
	*readPos += 4;
	printf("strf len: %d\n", dwValue);
	
	if(0 == flag)  //视频信息
	{
		/* strf info */
		BITMAPINFOHEADER bmiHeader;
		memset(&bmiHeader, 0, sizeof(BITMAPINFOHEADER));
		memcpy((char *)&bmiHeader, pInfoBuf + *readPos, sizeof(BITMAPINFOHEADER));
		*readPos += sizeof(BITMAPINFOHEADER);
		{
			printf("\n BITMAPINFOHEADER len %d\n", sizeof(BITMAPINFOHEADER));
			printf(" Size %d\n", bmiHeader.dwSize);
			printf(" Width %d\n", bmiHeader.dwWidth);
			printf(" Height %d\n", bmiHeader.dwHeight);
			printf(" Planes %d\n", bmiHeader.wPlanes);
			printf(" BitCount %d\n", bmiHeader.wBitCount);
			printf(" Compression %d\n", bmiHeader.dwCompression);
			printf(" SizeImage %d\n", bmiHeader.dwSizeImage);
			printf(" XPelsPerMeter %d\n", bmiHeader.dwXPelsPerMeter);
			printf(" YPelsPerMeter %d\n", bmiHeader.dwYPelsPerMeter);
			printf(" ClrUsed %d\n", bmiHeader.dwClrUsed);
			printf(" ClrImportant %d\n", bmiHeader.dwClrImportant);
		}

	}
	else
	{
		WAVEFORMAT waveFormat;
		memset(&waveFormat, 0, sizeof(WAVEFORMAT));
		memcpy((char *)&waveFormat, pInfoBuf + *readPos, sizeof(WAVEFORMAT));
		*readPos += sizeof(WAVEFORMAT);
		{
			printf("\n WAVEFORMAT len: \n", sizeof(WAVEFORMAT));
			printf("FormatTag: %d\n", waveFormat.wFormatTag);
			printf("Channels: %d\n", waveFormat.wChannels);
			printf("SamplesPerSec: %s\n", waveFormat.dwSamplesPerSec);
			printf("SamplesPerSec: %s\n", waveFormat.dwSamplesPerSec);
			printf("SamplesPerSec: %s\n", waveFormat.dwSamplesPerSec);
			printf("SamplesPerSec: %s\n", waveFormat.dwSamplesPerSec);
		}
		*readPos += waveFormat.wSize;
	}
}

int main(int argc, char *argv[])
{
	char fileName[32] = "1.avi";
	
	char buf[5];
	DWORD dwValue = 0;
	DWORD fileLen = 0;
	DWORD frameCount = 0;
	int hdrlListLen = 0;
	char *pInfo = NULL;
	int readPos = 0;
	DWORD moviPos = 0;
	DWORD offset = 0;

	FILE *fp_avi = fopen(fileName, "rb");			

	if(!fp_avi)
	{
		printf("open err\n");
		return -1;
	}

	FILE *fp_h264 = fopen("test.h264", "wb");
	if(!fp_h264)
	{
		printf("open h264 err\n");
		return -1;
	}

	fseek(fp_avi, 0, SEEK_SET);
	fseek(fp_avi, 0, SEEK_END);

	fileLen = ftell(fp_avi);

	fseek(fp_avi, 0, SEEK_SET);
	/* RIFF-4 字节 */
	memset(buf, 0, 5);
	fread(buf, 4, 1, fp_avi);
	printf("riffHead %s\n", buf);
		
	/* file Len-4字节: 文件总长度-8 */
	fread((char *)&dwValue, 4, 1, fp_avi);
	printf(" file len %d\n", dwValue);

	
	if(fileLen != (dwValue + 8))
	{
		printf("File is damaged!\n");
		return -1;
	}	
	
	/* file Type-4字节 */
	memset(buf, 0, 5);
	fread(buf, 4, 1, fp_avi);
	printf("file type: %s\n", buf);

	
	if(strncmp(buf, "AVI", 3))
	{
		printf("Isn't AVI file\n");
		return -1;
	}

	/* hdrl list-4字节 */
	memset(buf, 0, 5);
	fread(buf, 4, 1, fp_avi);
	printf("list: %s\n", buf);		

	/* list len-4字节 */
	dwValue = 0;
	fread((char *)&dwValue, 4, 1, fp_avi);
	printf("list len %d\n", dwValue);
	hdrlListLen = dwValue;
	pInfo = (char *)malloc(hdrlListLen +1);
	memset(pInfo, 0, hdrlListLen + 1);
	fread(pInfo, dwValue, 1, fp_avi);	
	{
		readPos = 0;
	
		/* list Type-4字节 */
		memset(buf, 0, 5);
		memcpy(buf, pInfo + readPos, 4);
		readPos += 4;
		printf("list type: %s\n", buf);
	
		/* avih-4字节 */
		memset(buf, 0, 5);
		memcpy(buf, pInfo+readPos, 4);
		readPos += 4;
		printf("avih: %s\n", buf);
		
		/* avih Len-4字节 */	
		dwValue = 0;
		memcpy((char *)&dwValue, pInfo+readPos, 4);
		readPos += 4;
		printf("avih len: %d\n", dwValue);
	
		/* avi header */
		MainAVIHeader mainAVIHeader;
		memset(&mainAVIHeader, 0, sizeof(MainAVIHeader));	
		memcpy((char *)&mainAVIHeader, pInfo + readPos, dwValue);
		readPos += dwValue;
		{
			printf("\n MainAVIHeader len: %d\n", sizeof(MainAVIHeader));
			printf("MicroSecPerFrame: %d\n", mainAVIHeader.dwMicroSecPerFrame);
			printf("MaxBytesPerSec: %d\n", mainAVIHeader.dwMaxBytesPerSec);
			printf("PaddingGranularity: %d\n", mainAVIHeader.dwPaddingGranularity);
			printf("Flages: %d\n", mainAVIHeader.dwFlages);
			printf("TotalFrame: %d\n", mainAVIHeader.dwTotalFrame);
			frameCount = mainAVIHeader.dwTotalFrame;
			printf("InitialFrames: %d\n", mainAVIHeader.dwInitialFrames);
			printf("Streams: %d\n", mainAVIHeader.dwStreams);
			printf("SuggestedBufferSize: %d\n", mainAVIHeader.dwSuggestedBufferSize);

			printf("Width: %d\n", mainAVIHeader.dwWidth);
			printf("Height: %d\n", mainAVIHeader.dwHeight);
		}
		if(!mainAVIHeader.dwFlages & AVIF_HASINDEX)
		{
			printf("Hasn't find idx info!\n");
			return -1;
		}

		/* list1 */
		printf("============List1==============\n");
		aviListInfo(pInfo, &readPos);

		/* list 2 */
		if(mainAVIHeader.dwStreams > 1)
		{
			int i;
			for(i = readPos; i<hdrlListLen;)
			{
				memset(buf, 0, 5);	
				memcpy(buf, pInfo + readPos, 4);
				printf("buf: %s\n", buf);
			
				if(!strncmp(buf, "LIST", 4))
					break;
				readPos += 4;
				i = readPos;
			}
	
			/* list2 */
			printf("\n =========List2==============\n");
			aviListInfo(pInfo, &readPos);
		}

		printf(" readPos= %d\n", readPos);
	}

	free(pInfo);
	pInfo = NULL;

	/* find list */
	{
		int i;
		for(i = readPos + 8; i<fileLen;)	
		{
			memset(buf, 0, 5);
			fread(buf, 4, 1, fp_avi);
			printf("buf %s\n", buf);

			if(!strncmp(buf, "LIST", 4))
			{
				/* len */
				dwValue = 0;
				fread((char *)&dwValue, 4, 1, fp_avi);
				i += 4;
				printf("len: %d\n", dwValue);
				
				/* type */
				memset(buf, 0, 5);
				fread(buf, 4, 1, fp_avi);
				i += 4;
				printf("type : %s\n", buf);
				

				fseek(fp_avi, offset + dwValue - 4, SEEK_CUR);
				if(!strncmp(buf, "movi", 4))
				{
					moviPos = ftell(fp_avi);
					moviPos = moviPos - dwValue + 4;
					break;
				}
			
			}
			else if(!strncmp(buf, "JUNK", 4))
			{
				/* len */
				dwValue = 0;
				fread((char *)&dwValue, 4, 1, fp_avi);
				printf("len : %d\n", dwValue);
				fseek(fp_avi, offset + dwValue, SEEK_CUR);
			}
			i += 4;
		}
		
	}	

	if(0 == moviPos)
		return -1;

	printf("moviPos %d\n", moviPos);

	printf("\n======== idxl info =============\n");
	
	/* idxl */
	memset(buf, 0, 5);
	fread(buf, 4, 1, fp_avi);
	printf(" idxl %s\n", buf);

	/* avih Len-4字节 */
	dwValue = 0;
	fread((char *)&dwValue, 4, 1, fp_avi);
	printf("idxl len: %d\n", dwValue);
	
	/* avi file index */
	
	AVI_IDX *pIndx = (AVI_IDX *)malloc(frameCount * sizeof(AVI_IDX));
	char pszBuf[1024 * 1024] = {0};
	DWORD nBufSize = 1024 * 1024;
	
	if(dwValue >= frameCount * sizeof(AVI_IDX))
		dwValue = frameCount * sizeof(AVI_IDX);

	fread((char *)pIndx, dwValue, 1, fp_avi);
	{
		int idx;
		for(idx = 0; idx < frameCount; ++idx)
		//for(idx = 0; idx < 10; ++idx)	
		{
			//memset(buf, 0, 5);
			//memcpy(buf, pIndx[idx].chunk_id, 4);
			//printf("\n id : %s\n", buf);					
			//printf(" key : %d\n", pIndx[idx].is_key);					
			//printf(" pos : %d\n", pIndx[idx].pos);					
			//printf(" size : %d\n", pIndx[idx].size);					

			//printf("moviPos %d\n", moviPos + pIndx[idx].pos);
			//printf("size %d\n", pIndx[idx].size);

			fseek(fp_avi, moviPos + pIndx[idx].pos, SEEK_SET);
			memset(pszBuf, 0, nBufSize);
			
			fread(pszBuf, pIndx[idx].size, 1, fp_avi);
			//printf("%s\n", pszBuf);	
			if(!strncmp(pIndx[idx].chunk_id, "00dc", 4));
				fwrite(pszBuf, pIndx[idx].size, 1, fp_h264);
		}
	
	}

	free(pIndx);	
	pIndx = NULL;
	close(fp_avi);
	close(fp_h264);
	return 0;

err:
	if(!fp_avi)
		close(fp_avi);
	if(!fp_h264)
		close(fp_h264);	
	return 0;
}
