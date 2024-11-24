#ifndef DOWNLOADPROGRESSWIDGET_H
#define DOWNLOADPROGRESSWIDGET_H

#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <tuple>
#include <unordered_map>

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>

using SessionId = uint32_t;
using TaskItem = std::tuple<std::string, uint64_t, uint64_t>;           //taskName, totalSize, downloadSize(0)
using TaskQLabelAndProgressBarPair = std::tuple<QLabel*, QProgressBar*>;

class DownloadProgressWidget : public QWidget
{
    Q_OBJECT
public:
    DownloadProgressWidget(QWidget *parent = nullptr);

    void freshFileInfo(const std::unordered_map<std::string, std::uint64_t>& fileInfo);
    bool hasOngoingTask()
    {
        return mainLayout->count() != 0;
    }

signals:
    void oneItemAdded();
    void oneItemFinished();

public slots:
    void addDownloadTask(const SessionId sessionId, TaskItem task);
    void removeDownloadTask(const SessionId sessionId, const std::string taskName = "", const std::string info = "");
    void updateDownloadProgress(const SessionId sessionId, uint64_t incrementSize);

private:
    QVBoxLayout *mainLayout;
    std::unordered_map<std::string, std::uint64_t> fileInfo_;

    std::mutex mtx_;
    std::map<SessionId, TaskItem> downloadingTasks_;
    std::map<SessionId, TaskQLabelAndProgressBarPair> downloadingTaskUi_;
};

#endif // DOWNLOADPROGRESSWIDGET_H
