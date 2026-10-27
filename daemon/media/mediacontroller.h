#ifndef MEDIACONTROLLER_H
#define MEDIACONTROLLER_H

#include <QHash>
#include <QObject>
#include "pulseaudiocontroller.h"

class QProcess;
class EarDetection;
class PlayerStatusWatcher;
class QDBusInterface;

class MediaController : public QObject
{
  Q_OBJECT
public:
  enum MediaState
  {
    Playing,
    Paused,
    Stopped
  };
  Q_ENUM(MediaState)
  enum EarDetectionBehavior
  {
    PauseWhenOneRemoved,
    PauseWhenBothRemoved,
    Disabled
  };
  Q_ENUM(EarDetectionBehavior)

  explicit MediaController(QObject *parent = nullptr);
  ~MediaController();

  void handleEarDetection(EarDetection*);
  void followMediaChanges();
  bool isActiveOutputDeviceAirPods();
  void handleConversationalAwareness(const QByteArray &data);
  void activateA2dpProfileWithRetry(const QString &macAddress);
  void cancelPendingA2dpActivation();
  void removeAudioOutputDevice();
  void setConnectedDeviceMacAddress(const QString &macAddress);
  bool isA2dpProfileAvailable();
  QString getPreferredA2dpProfile();
  QString getActiveProfile();
  // Both empty until PulseAudio has reported the AirPods card.
  QString activeAudioProfile() const;
  QString activeAudioCodec() const;
  bool restartWirePlumber();

  void setEarDetectionBehavior(EarDetectionBehavior behavior);
  inline EarDetectionBehavior getEarDetectionBehavior() const { return earDetectionBehavior; }

  void play();
  void pause();
  MediaState getCurrentMediaState() const;

Q_SIGNALS:
  void mediaStateChanged(MediaState state);

private:
  MediaState mediaStateFromPlayerctlOutput(const QString &output) const;
  QString getAudioDeviceName();
  QStringList getPlayingMediaPlayers();
  // Only the retry chain calls this, so the one-restart flag below cannot be read outside a chain.
  bool activateA2dpProfile();
  void attemptA2dpActivation(const QString &macAddress, quint64 generation, int attempt, int unanswered = 0);

  QStringList pausedByAppServices;
  int initialVolume = -1;
  QString connectedDeviceMacAddress;
  EarDetectionBehavior earDetectionBehavior = PauseWhenOneRemoved;
  QString m_deviceOutputName;
  PlayerStatusWatcher *playerStatusWatcher = nullptr;
  PulseAudioController *m_pulseAudio = nullptr;
  QString m_cachedA2dpProfile;
  struct CardProfile { QString name; QString codec; };
  // Keyed by card name, because the AirPods card can be reported before its address is known.
  QHash<QString, CardProfile> m_cardProfiles;
  quint64 m_earDetectionGeneration = 0;
  bool m_earOutPending = false;
  // A queued retry compares its captured generation against this, so a superseded chain stops.
  quint64 m_a2dpRetryGeneration = 0;
  bool m_wirePlumberRestartedThisChain = false;
};

#endif // MEDIACONTROLLER_H
