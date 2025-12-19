#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QTime>
#include <QDebug>
#include <QFileInfo>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_videoPlayer(nullptr)
    , m_musicPlayer(nullptr)
    , m_videoWidget(nullptr)
    , m_playButton(nullptr)
    , m_pauseButton(nullptr)
    , m_stopButton(nullptr)
    , m_prevButton(nullptr)
    , m_nextButton(nullptr)
    , m_positionSlider(nullptr)
    , m_positionLabel(nullptr)
    , m_durationLabel(nullptr)
    , m_songNameLabel(nullptr)
    , m_volumeSlider(nullptr)
    , m_playlistWidget(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("音乐播放器");

    // 初始化播放器
    m_videoPlayer = new QMediaPlayer(this);
    m_videoWidget = new QVideoWidget(this);
    m_videoPlayer->setVideoOutput(m_videoWidget);
    m_videoPlayer->setSource(QUrl("qrc:/background.mp4"));
    m_videoPlayer->setLoops(QMediaPlayer::Infinite);

    QAudioOutput *videoAudioOutput = new QAudioOutput(this);
    videoAudioOutput->setVolume(0);
    m_videoPlayer->setAudioOutput(videoAudioOutput);

    m_musicPlayer = new QMediaPlayer(this);
    QAudioOutput *musicAudioOutput = new QAudioOutput(this);
    musicAudioOutput->setVolume(0.5);
    m_musicPlayer->setAudioOutput(musicAudioOutput);

    createControls();
    setupLayout();
    setupMenu();

    connect(m_videoPlayer, &QMediaPlayer::errorOccurred, this, [](QMediaPlayer::Error error, const QString &errorString) {
        qWarning() << "视频错误:" << error << errorString;
    });

    connect(m_musicPlayer, &QMediaPlayer::errorOccurred, this, [](QMediaPlayer::Error error, const QString &errorString) {
        qWarning() << "音频错误:" << error << errorString;
    });

    connect(m_musicPlayer, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia && !m_playlist.isEmpty()) {
            nextMusic();
        }
    });

    m_videoPlayer->play();
}

void MainWindow::createControls()
{
    m_playButton = new QPushButton("播放", this);
    m_pauseButton = new QPushButton("暂停", this);
    m_stopButton = new QPushButton("停止", this);
    m_prevButton = new QPushButton("上一首", this);
    m_nextButton = new QPushButton("下一首", this);
    m_positionSlider = new QSlider(Qt::Horizontal, this);
    m_positionLabel = new QLabel("00:00", this);
    m_durationLabel = new QLabel("00:00", this);
    m_songNameLabel = new QLabel("未选择歌曲", this);
    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(50);
    m_playlistWidget = new QListWidget(this);

    // 设置样式
    QString buttonStyle = "QPushButton { background-color: rgba(50, 50, 50, 180); color: white; border: none; padding: 8px; }";
    m_playButton->setStyleSheet(buttonStyle);
    m_pauseButton->setStyleSheet(buttonStyle);
    m_stopButton->setStyleSheet(buttonStyle);
    m_prevButton->setStyleSheet(buttonStyle);
    m_nextButton->setStyleSheet(buttonStyle);

    m_positionLabel->setStyleSheet("color: white;");
    m_durationLabel->setStyleSheet("color: white;");
    m_songNameLabel->setStyleSheet("color: white; font-size: 16px; font-weight: bold;");
    m_playlistWidget->setStyleSheet("background-color: rgba(50, 50, 50, 180); color: white;");

    // 连接信号槽
    connect(m_playButton, &QPushButton::clicked, this, &MainWindow::playMusic);
    connect(m_pauseButton, &QPushButton::clicked, this, &MainWindow::pauseMusic);
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::stopMusic);
    connect(m_prevButton, &QPushButton::clicked, this, &MainWindow::prevMusic);
    connect(m_nextButton, &QPushButton::clicked, this, &MainWindow::nextMusic);
    connect(m_positionSlider, &QSlider::sliderMoved, this, &MainWindow::setMusicPosition);
    connect(m_musicPlayer, &QMediaPlayer::positionChanged, this, &MainWindow::updatePosition);
    connect(m_musicPlayer, &QMediaPlayer::durationChanged, this, &MainWindow::updateDuration);
    connect(m_volumeSlider, &QSlider::valueChanged, this, [this](int value) {
        m_musicPlayer->audioOutput()->setVolume(value / 100.0);
    });
    connect(m_playlistWidget, &QListWidget::itemClicked, this, &MainWindow::onPlaylistItemClicked);
}

void MainWindow::setupLayout()
{
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // 左侧播放列表区域
    QWidget *playlistContainer = new QWidget(this);
    QVBoxLayout *playlistLayout = new QVBoxLayout(playlistContainer);
    playlistLayout->addWidget(new QLabel("播放列表", this));
    playlistLayout->addWidget(m_playlistWidget);
    playlistContainer->setFixedWidth(250);

    // 右侧主区域
    QWidget *rightContainer = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightContainer);

    // 视频区域
    rightLayout->addWidget(m_videoWidget, 1);

    // 控制区域
    QWidget *controlContainer = new QWidget(this);
    controlContainer->setStyleSheet("background-color: rgba(50, 50, 50, 200);");
    QVBoxLayout *controlLayout = new QVBoxLayout(controlContainer);

    // 歌曲名显示
    controlLayout->addWidget(m_songNameLabel, 0, Qt::AlignCenter);

    // 进度条
    QHBoxLayout *progressLayout = new QHBoxLayout();
    progressLayout->addWidget(m_positionLabel);
    progressLayout->addWidget(m_positionSlider);
    progressLayout->addWidget(m_durationLabel);
    controlLayout->addLayout(progressLayout);

    // 控制按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_prevButton);
    buttonLayout->addWidget(m_playButton);
    buttonLayout->addWidget(m_pauseButton);
    buttonLayout->addWidget(m_stopButton);
    buttonLayout->addWidget(m_nextButton);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(new QLabel("音量:", this));
    buttonLayout->addWidget(m_volumeSlider);
    controlLayout->addLayout(buttonLayout);

    rightLayout->addWidget(controlContainer);
    mainLayout->addWidget(playlistContainer);
    mainLayout->addWidget(rightContainer, 1);

    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);
}

void MainWindow::setupMenu()
{
    QMenu *playlistMenu = menuBar()->addMenu("播放列表");

    QAction *newAction = new QAction("新建播放列表", this);
    QAction *loadAction = new QAction("加载播放列表", this);
    QAction *saveAction = new QAction("保存播放列表", this);
    QAction *clearAction = new QAction("清空播放列表", this);

    playlistMenu->addAction(newAction);
    playlistMenu->addAction(loadAction);
    playlistMenu->addAction(saveAction);
    playlistMenu->addAction(clearAction);

    connect(newAction, &QAction::triggered, this, &MainWindow::createNewPlaylist);
    connect(loadAction, &QAction::triggered, this, &MainWindow::loadPlaylist);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveCurrentPlaylist);
    connect(clearAction, &QAction::triggered, this, &MainWindow::clearPlaylist);
}

void MainWindow::updateSongName(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString songName = fileInfo.fileName();
    m_songNameLabel->setText(songName);
}









void MainWindow::playMusic()
{
    if (m_musicPlayer->source().isEmpty()) {
        QString fileName = QFileDialog::getOpenFileName(this, "打开音乐文件", "", "音频文件 (*.mp3 *.wav *.ogg)");
        if (!fileName.isEmpty()) {
            m_musicPlayer->setSource(QUrl::fromLocalFile(fileName));
            updateSongName(fileName);  // 更新歌曲名
        }
    }
    m_musicPlayer->play();
}

void MainWindow::pauseMusic()
{
    m_musicPlayer->pause();
}

void MainWindow::stopMusic()
{
    m_musicPlayer->stop();
    m_positionSlider->setValue(0);
    m_positionLabel->setText("00:00");
}

void MainWindow::setMusicPosition(int position)
{
    m_musicPlayer->setPosition(position);
}

void MainWindow::updatePosition(qint64 position)
{
    if (!m_positionSlider->isSliderDown()) {
        m_positionSlider->setValue(position);
    }
    QTime currentTime(0, 0, 0);
    currentTime = currentTime.addMSecs(position);
    m_positionLabel->setText(currentTime.toString("mm:ss"));
}

void MainWindow::updateDuration(qint64 duration)
{
    m_positionSlider->setRange(0, duration);
    QTime totalTime(0, 0, 0);
    totalTime = totalTime.addMSecs(duration);
    m_durationLabel->setText(totalTime.toString("mm:ss"));
}

void MainWindow::playTrack(int index)
{
    if (index >= 0 && index < m_playlist.size()) {
        m_currentTrackIndex = index;
        QString filePath = m_playlist.at(index);
        m_musicPlayer->setSource(QUrl::fromLocalFile(filePath));
        updateSongName(filePath);
        m_musicPlayer->play();
        m_playlistWidget->setCurrentRow(index);
    }
}

void MainWindow::prevMusic()
{
    if (m_playlist.isEmpty()) return;

    int newIndex = m_currentTrackIndex - 1;
    if (newIndex < 0) newIndex = m_playlist.size() - 1;

    playTrack(newIndex);
}

void MainWindow::nextMusic()
{
    if (m_playlist.isEmpty()) return;

    int newIndex = m_currentTrackIndex + 1;
    if (newIndex >= m_playlist.size()) newIndex = 0;

    playTrack(newIndex);
}

void MainWindow::onPlaylistItemClicked(QListWidgetItem *item)
{
    int index = m_playlistWidget->row(item);
    playTrack(index);
}

void MainWindow::createNewPlaylist()
{
    QStringList files = QFileDialog::getOpenFileNames(this, "选择音乐文件", "", "音频文件 (*.mp3 *.wav *.ogg)");
    if (!files.isEmpty()) {
        m_playlist = files;
        m_playlistWidget->clear();
        m_playlistWidget->addItems(files);
        m_currentPlaylistName = QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
    }
}

void MainWindow::saveCurrentPlaylist()
{
    if (m_playlist.isEmpty()) {
        QMessageBox::warning(this, "警告", "播放列表为空，无法保存");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this, "保存播放列表", m_currentPlaylistName, "播放列表文件 (*.m3u)");
    if (!filename.isEmpty()) {
        savePlaylistToFile(filename);
        m_currentPlaylistName = QFileInfo(filename).baseName();
    }
}

void MainWindow::loadPlaylist()
{
    QString filename = QFileDialog::getOpenFileName(this, "加载播放列表", "", "播放列表文件 (*.m3u)");
    if (!filename.isEmpty()) {
        loadPlaylistFromFile(filename);
        m_currentPlaylistName = QFileInfo(filename).baseName();
    }
}

void MainWindow::clearPlaylist()
{
    m_playlist.clear();
    m_playlistWidget->clear();
    m_currentTrackIndex = -1;
    m_musicPlayer->stop();
    m_songNameLabel->setText("未选择歌曲");
}

void MainWindow::savePlaylistToFile(const QString &filename)
{
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        foreach (const QString &track, m_playlist) {
            out << track << "\n";
        }
        file.close();
    }
}

void MainWindow::loadPlaylistFromFile(const QString &filename)
{
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_playlist.clear();
        m_playlistWidget->clear();

        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty() && QFile::exists(line)) {
                m_playlist.append(line);
                m_playlistWidget->addItem(line);
            }
        }
        file.close();
    }
}

MainWindow::~MainWindow()
{
    delete m_videoPlayer;
    delete m_musicPlayer;
    delete m_videoWidget;
    delete ui;
}
