






extern "C" {
#include "libavformat/avformat.h"
}


#include "inputparser.h"
#include "Utils/ylogger.h"


YDemuxer::YDemuxer(const std::string &url)
{
    m_url = url;
    m_stop = false;

    connect(this, &YDemuxer::signal_startParser,
            this, &YDemuxer::slot_startParser, Qt::QueuedConnection);
}

YDemuxer::~YDemuxer()
{

}

void YDemuxer::startParser()
{
    emit signal_startParser();
}

void YDemuxer::stopParser()
{
    m_stop = true;
}

void YDemuxer::slot_startParser()
{
    AVDictionary *options = nullptr;
    av_dict_set(&options, "rtsp_transport", "tcp", 0);
    av_dict_set(&options, "stimeout", "5000000", 0);

    AVFormatContext *fc = avformat_alloc_context();
    fc->max_delay = 500;

    int m_ret = avformat_open_input(&fc, m_url.c_str(), nullptr, &options);
    if (nullptr != options) {
        av_dict_free(&options);
    }

    std::string error_string;
    char m_err_str[128];
    if (m_ret < 0) {
        av_make_error_string(m_err_str, sizeof(m_err_str), m_ret);
        error_string.assign(m_err_str);
        YLOG(Module_VideoIo, LoggerSeverity_WARNING) << "Failed to parse " << m_url
            << " due to " << error_string << "!";
        emit signal_parseFailed(error_string);
        return;
    }

    m_ret = avformat_find_stream_info(fc, NULL);
    if (m_ret < 0) {
        avformat_close_input(&fc);
        av_make_error_string(m_err_str, sizeof(m_err_str), m_ret);
        error_string.assign(m_err_str);
        YLOG(Module_VideoIo, LoggerSeverity_WARNING) << "Failed to parse " << m_url
            << " due to " << error_string << "!";
        emit signal_parseFailed(error_string);
        return;
    }

    int video_stream_index = -1;
    for (unsigned int i = 0; i < fc->nb_streams; i++) {
        if (AVMEDIA_TYPE_VIDEO == fc->streams[i]->codecpar->codec_type) {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index < 0) {
        avformat_close_input(&fc);
        snprintf(m_err_str, sizeof(m_err_str), "No video stream found in specified url %s!", m_url.c_str());
        error_string.assign(m_err_str);
        YLOG(Module_VideoIo, LoggerSeverity_WARNING) << "Failed to parse " << m_url
            << " due to " << error_string << "!";
        emit signal_parseFailed(error_string);
        return;
    }

    emit signal_parseOk();
    YLOG(Module_VideoIo, LoggerSeverity_INFO) << "Success to open " << m_url << "!";
//    AVCodecContext *ctx = fc->streams[video_stream_index]->codec;
//    emitSignalSetDecoder(ctx->codec_id, ctx->extradata_size, ctx->extradata);

    /* read packet */
    AVPacket *pkt = av_packet_alloc();
    while (true) {
        m_ret = av_read_frame(fc, pkt);
        if(m_ret < 0) {
            av_make_error_string(m_err_str, sizeof(m_err_str), m_ret);
            break;
        }

        if (pkt->stream_index == video_stream_index) {
//            SharedReusableAvPacket *sp = m_pkt_pool->getFreeBuffer();
//            sp->resize(pkt->size);
//            memcpy(sp->getAvPacket()->data, pkt->data, pkt->size);
//            sp->getAvPacket()->size = pkt->size;
//            emit signal_packetReady(sp);
        }

        av_packet_unref(pkt);
        if (m_stop) {
            break;
        }
    }

    /* parser is done and free resource */
    av_packet_free(&pkt);
    avformat_close_input(&fc);

    if(m_stop) {
        YLOG(Module_VideoIo, LoggerSeverity_INFO) << "Success to close " << m_url << "!";
    } else {
        snprintf(m_err_str, sizeof(m_err_str), "End of file");
        error_string.assign(m_err_str);
        YLOG(Module_VideoIo, LoggerSeverity_WARNING) << "Failed to parse " << m_url
            << " due to " << error_string << "!";
        emit signal_parseFailed(error_string);
    }
}

InputParser::InputParser(const std::string &url, int cid, QObject *parent)
    : QThread(parent)
{
    m_cid = cid;
    m_url = url;
    m_worker = new YDemuxer(url);
    m_worker->moveToThread(this);
}

InputParser::~InputParser()
{
    delete m_worker;
}

void InputParser::startParser()
{
    start();
    m_worker->startParser();
}

void InputParser::stopParser()
{
    m_worker->stopParser();
    quit();
    wait();

    /* LOG */
}

void InputParser::slot_parseOk()
{
    emit signal_parseOk(m_cid);
}

void InputParser::slot_parserFailed(std::string error_string)
{
    emit signal_parseFailed(m_cid, error_string);
    m_worker->startParser();
}
