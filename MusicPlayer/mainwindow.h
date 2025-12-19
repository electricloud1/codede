#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QAudioOutput>
#include <QListWidget>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void playMusic();
    void pauseMusic();
    void stopMusic();
    void prevMusic();
    void nextMusic();
    void setMusicPosition(int position);
    void updatePosition(qint64 position);
    void updateDuration(qint64 duration);
    void onPlaylistItemClicked(QListWidgetItem *item);
    void createNewPlaylist();
    void saveCurrentPlaylist();
    void loadPlaylist();
    void clearPlaylist();

private:
    Ui::MainWindow *ui;
    QMediaPlayer *m_videoPlayer;
    QMediaPlayer *m_musicPlayer;
    QVideoWidget *m_videoWidget;

    // 控件成员
    QPushButton *m_playButton;
    QPushButton *m_pauseButton;
    QPushButton *m_stopButton;
    QPushButton *m_prevButton;
    QPushButton *m_nextButton;
    QSlider *m_positionSlider;
    QLabel *m_positionLabel;
    QLabel *m_durationLabel;
    QLabel *m_songNameLabel;
    QSlider *m_volumeSlider;
    QListWidget *m_playlistWidget;

    // 播放列表相关
    QStringList m_playlist;
    int m_currentTrackIndex = -1;
    QString m_currentPlaylistName;

    void createControls();
    void setupLayout();
    void setupMenu();
    void updateSongName(const QString &filePath);
    void playTrack(int index);
    void savePlaylistToFile(const QString &filename);
    void loadPlaylistFromFile(const QString &filename);
};

#endif // MAINWINDOW_H
