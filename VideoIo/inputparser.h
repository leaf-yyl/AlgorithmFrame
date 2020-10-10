






#ifndef INPUTPARSER_H
#define INPUTPARSER_H

extern "C"{
#include "libavcodec/avcodec.h"
}

#include <QThread>
#include <QString>

#include "Utils/yworker.h"

class YDemuxer : public QObject
{
    Q_OBJECT
public:
    explicit YDemuxer(const std::string& url);
    virtual ~YDemuxer();

    void startParser();
    void stopParser();

protected slots:
    void slot_startParser();

signals:
    void signal_startParser();

    void signal_parseOk();
    void signal_parseFailed(std::string);

    void signal_packetReady();

private:
    std::string m_url;
    std::string m_connection_type;

    volatile bool m_stop;
};

class YDecoder : public QObject
{
    Q_OBJECT
public:
    explicit YDecoder(const std::string& url);
    virtual ~YDecoder();

protected slots:
    void slot_startParser();

signals:
    void signal_startParser();

    void signal_parseOk();
    void signal_parseFailed(std::string);

    void signal_packetReady();

private:
    std::string m_url;
    volatile bool m_stop;
};

class InputParser : public QObject
{
    Q_OBJECT
public:
    explicit InputParser(const std::string& url, int cid, QObject *parent = nullptr);
    ~InputParser();

    YDemuxer* createDemuxer(const std::string& demuxer_name);
    YDecoder* createDecoder(const std::string& decoder_name);

    void startParser(YDemuxer* demuxer, YDecoder* decoder);
    void stopParser();

protected slots:
    void slot_parseOk();
    void slot_parserFailed(std::string error_string);

signals:
    void signal_parseOk(int);
    void signal_parseFailed(int, std::string);

private:
    int m_cid;
    std::string m_url;
    YDemuxer* m_worker;
};

#endif // INPUTPARSER_H
