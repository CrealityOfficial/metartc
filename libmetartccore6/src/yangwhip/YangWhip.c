//
// Copyright (c) 2019-2022 yanggaofeng
//

#include <yangwhip/YangWhip.h>

#include <yangrtc/YangRtcConnection.h>

#include <yangice/YangRtcStun.h>
#include <yangice/YangRtcSocket.h>

#include <yangsdp/YangSdp.h>

#include <yangutil/sys/YangLog.h>
#include <yangutil/sys/YangCUrl.h>
#include <yangutil/sys/YangCString.h>
#include <yangutil/sys/YangHttp.h>

#define Yang_SDP_BUFFERLEN 1024*12
unsigned char* base64_encode(unsigned char* str)
{
	long len;
	long str_len;
	unsigned char* res;
	int i, j;
	//定义base64编码表  
	unsigned char* base64_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	//计算经过base64编码后的字符串长度  
	str_len = strlen(str);
	if (str_len % 3 == 0)
		len = str_len / 3 * 4;
	else
		len = (str_len / 3 + 1) * 4;

	res = malloc(sizeof(unsigned char) * len + 1);
	res[len] = '\0';

	//以3个8位字符为一组进行编码  
	for (i = 0, j = 0; i < len - 2; j += 3, i += 4)
	{
		res[i] = base64_table[str[j] >> 2]; //取出第一个字符的前6位并找出对应的结果字符  
		res[i + 1] = base64_table[(str[j] & 0x3) << 4 | (str[j + 1] >> 4)]; //将第一个字符的后位与第二个字符的前4位进行组合并找到对应的结果字符  
		res[i + 2] = base64_table[(str[j + 1] & 0xf) << 2 | (str[j + 2] >> 6)]; //将第二个字符的后4位与第三个字符的前2位组合并找出对应的结果字符  
		res[i + 3] = base64_table[str[j + 2] & 0x3f]; //取出第三个字符的后6位并找出结果字符  
	}

	switch (str_len % 3)
	{
	case 1:
		res[i - 2] = '=';
		res[i - 1] = '=';
		break;
	case 2:
		res[i - 1] = '=';
		break;
	}

	return res;
}
unsigned char* base64_decode(unsigned char* code)
{
	//根据base64表，以字符找到对应的十进制数据  
	int table[] = { 0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,0,0,0,0,0,
			 0,0,0,0,0,0,0,62,0,0,0,
			 63,52,53,54,55,56,57,58,
			 59,60,61,0,0,0,0,0,0,0,0,
			 1,2,3,4,5,6,7,8,9,10,11,12,
			 13,14,15,16,17,18,19,20,21,
			 22,23,24,25,0,0,0,0,0,0,26,
			 27,28,29,30,31,32,33,34,35,
			 36,37,38,39,40,41,42,43,44,
			 45,46,47,48,49,50,51
	};
	long len;
	long str_len;
	unsigned char* res;
	int i, j;

	//计算解码后的字符串长度  
	len = strlen(code);
	//判断编码后的字符串后是否有=  
	if (strstr(code, "=="))
		str_len = len / 4 * 3 - 2;
	else if (strstr(code, "="))
		str_len = len / 4 * 3 - 1;
	else
		str_len = len / 4 * 3;

	res = malloc(sizeof(unsigned char) * str_len + 1);
	res[str_len] = '\0';

	//以4个字符为一位进行解码  
	for (i = 0, j = 0; i < len - 2; j += 3, i += 4)
	{
		res[j] = ((unsigned char)table[code[i]]) << 2 | (((unsigned char)table[code[i + 1]]) >> 4); //取出第一个字符对应base64表的十进制数的前6位与第二个字符对应base64表的十进制数的后2位进行组合  
		res[j + 1] = (((unsigned char)table[code[i + 1]]) << 4) | (((unsigned char)table[code[i + 2]]) >> 2); //取出第二个字符对应base64表的十进制数的后4位与第三个字符对应bas464表的十进制数的后4位进行组合  
		res[j + 2] = (((unsigned char)table[code[i + 2]]) << 6) | ((unsigned char)table[code[i + 3]]); //取出第三个字符对应base64表的十进制数的后2位与第4个字符进行组合  
	}

	return res;

}
#if 1
int32_t yang_whip_getSignal(YangRtcSession* session,char* url,char** premoteSdp,char* localSdp) {
	int32_t err=Yang_Ok;
	YangUrlData urlData={0};
	yang_http_url_parse(session->context.avinfo->sys.familyType,url,&urlData);

	char* remoteSdp=(char*)yang_calloc(1,Yang_SDP_BUFFERLEN);
	char* postdata = (char*)yang_calloc(1, Yang_SDP_BUFFERLEN);
	sprintf(postdata, "{\"type\":\"offer\", \"sdp\":\"%s\" }", localSdp);
	postdata = base64_encode(postdata);
	if(yang_http_post(yangtrue,session->context.avinfo->sys.familyType,remoteSdp,urlData.server,
			urlData.port, urlData.stream, (uint8_t*)postdata, yang_strlen(postdata))){
		char* endp=yang_strstr(remoteSdp,"\r\n\r\n");
		if(endp) yang_error("\n%s",endp+4);
		yang_free(remoteSdp);
		return yang_error_wrap(1,"query whip sdp failure!");
	}

	if(yang_strstr(remoteSdp,"200")){
		char* p2=yang_strstr(remoteSdp,"\r\n\r\n");
		if(p2==NULL) return ERROR_RTC_Whip;
		p2 = base64_decode(p2+4);
		char* p=yang_strstr(p2,"v=0");
		if(p){
			int32_t len=yang_strlen(p);
			char* sdp=(char*)yang_calloc(len+1,1);
			p[len-2] = 0;
			yang_cstr_replace(p,sdp, "\\r\\n", "\n");
			p=yang_strstr(sdp,"\n\n");
			if(p) *p=0;
			*premoteSdp=sdp;
			err=Yang_Ok;
		}
	}else{
		err=ERROR_RTC_Whip;
	}
	return err;

}
#else
int32_t yang_whip_getSignal(YangRtcSession* session,char* url,char** premoteSdp,char* localSdp) {
	int32_t err=Yang_Ok;
	YangUrlData urlData={0};
	yang_http_url_parse(session->context.avinfo->sys.familyType,url,&urlData);

	char* remoteSdp=(char*)yang_calloc(1,Yang_SDP_BUFFERLEN);

	if(yang_http_post(yangtrue,session->context.avinfo->sys.familyType,remoteSdp,urlData.server,
			urlData.port, urlData.stream, (uint8_t*)localSdp, yang_strlen(localSdp))){
		char* endp=yang_strstr(remoteSdp,"\r\n\r\n");
		if(endp) yang_error("\n%s",endp+4);
		yang_free(remoteSdp);
		return yang_error_wrap(1,"query whip sdp failure!");
	}

	if(yang_strstr(remoteSdp,"201")&&yang_strstr(remoteSdp,"Created")){
		char* p2=yang_strstr(remoteSdp,"\r\n\r\n");
		if(p2==NULL) return ERROR_RTC_Whip;
		char* p=yang_strstr(p2,"v=0");
		if(p){
			int32_t len=yang_strlen(p);
			char* sdp=(char*)yang_calloc(len+1,1);
			yang_cstr_replace(p,sdp, "\r\n", "\n");
			p=yang_strstr(sdp,"\n\n");
			if(p) *p=0;
			*premoteSdp=sdp;
			err=Yang_Ok;
		}
	}else{
		err=ERROR_RTC_Whip;
	}
	return err;

}
#endif
int32_t yang_whip_connectPeer(YangRtcConnection* conn,char* url){
	YangRtcSession* session=conn->session;
	int err=Yang_Ok;
	char* remoteSdp=NULL;


	char *localSdp=NULL;
	conn->createOffer(session, &localSdp);

	if ((err=yang_whip_getSignal(conn->session,url,&remoteSdp,localSdp))  == Yang_Ok) {
		if(remoteSdp) conn->setRemoteDescription(conn->session,remoteSdp);
	}
	yang_free(localSdp);
	yang_free(remoteSdp);
    return err;
}
