






#ifndef YWORKER_H
#define YWORKER_H

#include <QObject>
#include <QThread>

#include "ylogger.h"

/* 后台线程工作者基类，通过接收主线程的signal，在后台线程执行相应的slot.
 * 提供了create/release接口，以实现后台线程工作者完成线程相关的初始化和释放工作
 */
class YWorker : public QObject
{
    Q_OBJECT
public:
    YWorker(std::string name = "") {
        m_initialized = false;
        m_work_name = name;
        connect(this, &YWorker::signal_doCreate, this, &YWorker::slot_create, Qt::QueuedConnection);
        connect(this, &YWorker::signal_doRelease, this, &YWorker::slot_release, Qt::QueuedConnection);
    }
    virtual ~YWorker() {
        if(m_initialized) {
            YLOG(Module_Utils, LoggerSeverity_WARNING)
                << "Worker " << m_work_name
                << " is not released, try to release it on host thread!";
            release();
            m_initialized = false;
        }
    }

    void createOnBackground() {
        emit signal_doCreate();
    }

    void releaseOnBackground() {
        emit signal_doRelease();
    }

protected:
    virtual void create() {}
    virtual void release() {}

signals:
    /* signals received from other objects */
    void signal_doCreate();
    void signal_doRelease();

    /* signals emitted to other objects */
    void signal_workerCreated(YWorker *worker);

protected slots:
    void slot_create() {
        if (!m_initialized) {
            create();
            m_initialized = true;
            emit signal_workerCreated(this);
        }
    }

    void slot_release() {
        if (m_initialized) {
            release();
            m_initialized = false;
        }
    }

private:
    bool m_initialized;
    std::string m_work_name;
};

#endif // YWORKER_H
