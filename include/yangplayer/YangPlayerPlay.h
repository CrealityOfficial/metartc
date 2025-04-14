//
// Copyright (c) 2019-2022 yanggaofeng
//

#ifndef YANGAPP_YANGPLAYAPP_H_
#define YANGAPP_YANGPLAYAPP_H_
#include <yangutil/buffer/YangAudioPlayBuffer.h>
#include <yangutil/buffer/YangVideoBuffer.h>

#include <yangavutil/audio/YangRtcAec.h>

#include <yangdecoder/YangVideoDecoderHandles.h>
#ifdef __linux__
#include <yangaudiodev/linux/YangAudioPlayLinux.h>
#endif
#ifdef _WIN32
#include <yangaudiodev/YangWinAudioApiRender.h>
#endif


#include <vector>

using namespace std;
class YangPlayerPlay {
public:
	YangPlayerPlay();
	virtual ~YangPlayerPlay();
    void initAudioPlay(YangContext* paudio);
	void startAudioPlay();
	void setInAudioList(YangAudioPlayBuffer *paudioList);
	void stopAll();
private:
#ifdef _WIN32
        YangWinAudioApiRender *m_audioPlay;
#else

#ifdef __ANDROID__
        YangAudioPlayAndroid *m_audioPlay;
#else
        YangAudioPlayLinux *m_audioPlay;
#endif


#endif
	int32_t vm_audio_player_start;

};

#endif /* YANGAPP_YANGPLAYAPP_H_ */
