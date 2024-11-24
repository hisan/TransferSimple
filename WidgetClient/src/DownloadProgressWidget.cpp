#include <stack>
#include <cassert>

#include "DownloadProgressWidget.hpp"

DownloadProgressWidget::DownloadProgressWidget(QWidget *parent)
    : QWidget(parent)
    , mainLayout(new QVBoxLayout(this))
{
    setFixedWidth(200);
    setMinimumHeight(100);
    this->setWindowFlags(this->windowFlags()|Qt::WindowStaysOnTopHint);
    setLayout(mainLayout);
}

void DownloadProgressWidget::addDownloadTask(const SessionId sessionId, TaskItem task)
{
    if (downloadingTasks_.count(sessionId))
        return;
    
    std::get<1>(task) = (std::uint64_t)fileInfo_[std::get<0>(task)];

    {
        std::lock_guard<std::mutex> lk(mtx_);
        downloadingTasks_[sessionId] = task;
    }

    auto taskName = std::get<0>(task);
    QLabel *taskLabel = new QLabel(taskName.c_str(), this);
    taskLabel->setText(QString("%1 (%2%)").arg(taskName.c_str()).arg(0));

    QProgressBar *progressBar = new QProgressBar(this);
    progressBar->setValue(0);
    progressBar->setRange(0, 100);
    progressBar->setMinimumWidth(150);
    progressBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    mainLayout->addWidget(taskLabel);
    mainLayout->addWidget(progressBar);
    this->adjustSize();

    downloadingTaskUi_.insert({sessionId, TaskQLabelAndProgressBarPair(taskLabel, progressBar)});
    emit oneItemAdded();
}

void DownloadProgressWidget::removeDownloadTask(const SessionId sessionId, const std::string taskName, const std::string info)
{
    std::stack<std::tuple<SessionId, TaskQLabelAndProgressBarPair>> waitRelayoutProgressPair;

    {
        std::lock_guard<std::mutex> lk(mtx_);
        if (!downloadingTasks_.count(sessionId) || !downloadingTaskUi_.count(sessionId))
            return;

        auto& waitDelTuple = downloadingTaskUi_[sessionId];

        int cnt = mainLayout->count();
        assert(0 == (cnt%2));
        for (int i = cnt-1; i > 0; i -= 2)
        {
            auto taskLabel = dynamic_cast<QLabel*>(mainLayout->itemAt(i-1)->widget());
            auto progressBar = dynamic_cast<QProgressBar*>(mainLayout->itemAt(i)->widget());

            taskLabel->setParent(nullptr);
            progressBar->setParent(nullptr);

            mainLayout->removeWidget(taskLabel);
            mainLayout->removeWidget(progressBar);

            auto targetPair = std::make_tuple(taskLabel, progressBar);
            if (targetPair == waitDelTuple)
            {
                delete taskLabel;
                delete progressBar;
                downloadingTasks_.erase(sessionId);
                downloadingTaskUi_.erase(sessionId);

                emit oneItemFinished();
                continue;
            }
            waitRelayoutProgressPair.push({sessionId, targetPair});
        }
    }

    while (not waitRelayoutProgressPair.empty())
    {
        auto& [sId, taskQLabelAndProgressBarPair] = waitRelayoutProgressPair.top();
        auto& [taskLabel, progressBar] = taskQLabelAndProgressBarPair;
        waitRelayoutProgressPair.pop();

        taskLabel->setParent(this);
        progressBar->setParent(this);

        mainLayout->addWidget(taskLabel);
        mainLayout->addWidget(progressBar);
    }
    this->adjustSize();
}

void DownloadProgressWidget::updateDownloadProgress(const SessionId sessionId, uint64_t incrementSize)
{
    int progress = 0;

    {
        std::lock_guard<std::mutex> lk(mtx_);
        if (!downloadingTasks_.count(sessionId))
            return;

        auto& [taskName, totalSize, downloadedSize] = downloadingTasks_[sessionId];
        auto& [taskLabel, progressBar] = downloadingTaskUi_[sessionId];

        downloadedSize += incrementSize;
        progress = totalSize > 0 ? static_cast<int>((downloadedSize * 100) / totalSize) : 0;

        progressBar->setValue(progress);
        taskLabel->setText(QString("%1 (%2%)").arg(taskName.c_str()).arg(progress));
    }

    if (progress == 100)
        removeDownloadTask(sessionId);
}

void DownloadProgressWidget::freshFileInfo(const std::unordered_map<std::string, std::uint64_t>& fileInfo)
{
    fileInfo_ = fileInfo;
}
