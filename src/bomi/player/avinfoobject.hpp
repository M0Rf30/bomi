#ifndef AVINFOOBJECT_HPP
#define AVINFOOBJECT_HPP

#include "enum/colorrange.hpp"
#include "enum/colorspace.hpp"

class AudioFormat;                      class StreamTrack;
class StreamList;

class CodecObject : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString family READ family NOTIFY familyChanged)
    Q_PROPERTY(QString type READ type NOTIFY typeChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
public:
    auto parse(const QString &info) -> void;
    auto family() const -> QString { return m_family.toUpper(); }
    auto type() const -> QString { return m_type; }
    auto description() const -> QString { return m_desc; }
    auto setFamily(const QString &family) -> void
        { if (_Change(m_family, family)) emit familyChanged(); }
    auto setType(const QString &type) -> void
        { if (_Change(m_type, type)) emit typeChanged(); }
signals:
    void familyChanged();
    void typeChanged();
    void descriptionChanged();
private:
    QString m_family, m_type, m_desc;
};

class AvCommonFormatObject : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString type READ type NOTIFY typeChanged)
    Q_PROPERTY(int bitrate READ bitrate NOTIFY bitrateChanged)
    Q_PROPERTY(int depth READ depth NOTIFY depthChanged)
public:
    auto type() const -> QString { return m_type; }
    auto bitrate() const -> int { return m_bitrate; }
    auto setType(const QString &type) -> void
        { if (_Change(m_type, type.toUpper())) emit typeChanged(); }
    auto setBitrate(int bps) -> void
        { if (_Change(m_bitrate, bps)) emit bitrateChanged(); }
    auto depth() const -> int { return m_depth; }
    auto setDepth(int depth) -> void
        { if (_Change(m_depth, depth)) emit depthChanged(); }
signals:
    void typeChanged();
    void bitrateChanged();
    void depthChanged();
private:
    QString m_type;
    int m_bitrate = 0, m_depth = 0;
};

class AvTrackObject : public QObject {
    Q_OBJECT
    Q_PROPERTY(int number READ number CONSTANT FINAL)
    Q_PROPERTY(QString title READ title CONSTANT FINAL)
    Q_PROPERTY(QString language READ language CONSTANT FINAL)
    Q_PROPERTY(bool valid READ isValid CONSTANT FINAL)
    Q_PROPERTY(bool selected READ isSelected CONSTANT FINAL)
    Q_PROPERTY(QString codec READ codec CONSTANT FINAL)
public:
    AvTrackObject() = default;
    auto id() const -> int {return m_id;}
    auto number() const -> int { return m_number; }
    auto language() const -> QString { return m_lang; }
    auto title() const -> QString { return m_title; }
    auto isValid() const -> bool { return m_id > 0; }
    auto codec() const -> QString { return m_codec; }
    auto isSelected() const -> bool { return m_selected; }
    static auto fromTrack(int n, const StreamTrack &track) -> AvTrackObject*;
private:
    int m_id = -1, m_number = -1; QString m_title, m_lang, m_codec;
    bool m_selected = false;
};

class AvCommonObject : public QObject {
    Q_OBJECT
    Q_PROPERTY(CodecObject *codec READ codec CONSTANT FINAL)
    Q_PROPERTY(AvTrackObject *track READ track NOTIFY trackChanged)
    Q_PROPERTY(QQmlListProperty<AvTrackObject> tracks READ tracks NOTIFY tracksChanged)
public:
    auto tracks() const -> QQmlListProperty<AvTrackObject>;
    auto track() const -> AvTrackObject*;
    auto codec() const -> const CodecObject* { return &m_codec; }
    auto codec() -> CodecObject* { return &m_codec; }
signals:
    void tracksChanged();
    void trackChanged();
private:
    auto update(const StreamList &tracks, bool clear = true) -> AvTrackObject*;
    auto setTracks(const StreamList &tracks) -> void;
    auto setTracks(const StreamList &tracks1, const StreamList &tracks2) -> void;
    friend class PlayEngine;
    CodecObject m_codec;
    QVector<AvTrackObject*> m_tracks;
    AvTrackObject *m_track = nullptr;
};

/******************************************************************************/

class AudioFormatObject : public AvCommonFormatObject {
    Q_OBJECT
    Q_PROPERTY(int samplerate READ samplerate NOTIFY samplerateChanged)
    Q_PROPERTY(QString channels READ channels NOTIFY channelsChanged)
public:
    auto samplerate() const -> int { return m_srate; }
    auto channels() const -> QString { return m_ch; }
    auto setFormat(const AudioFormat &format) -> void;
    auto setSampleRate(int s, bool bitrate) -> void
    {
        if (_Change(m_srate, s)) {
            emit samplerateChanged();
            if (bitrate)
                setBitrate(m_srate * m_nch * depth());
        }
    }
    auto setChannels(const QString &ch, int n) -> void
        { if (_Change(m_ch, ch) | _Change(m_nch, n)) emit channelsChanged(); }
signals:
    void samplerateChanged();
    void channelsChanged();
private:
    int m_srate = 0, m_nch = 0;
    QString m_ch;
};

class AudioObject : public AvCommonObject {
    Q_OBJECT
    Q_PROPERTY(AudioFormatObject *input READ input CONSTANT FINAL)
    Q_PROPERTY(AudioFormatObject *output READ output CONSTANT FINAL)
    Q_PROPERTY(AudioFormatObject *renderer READ renderer CONSTANT FINAL)
    Q_PROPERTY(double normalizer READ normalizer NOTIFY normalizerChanged)
    Q_PROPERTY(QString driver READ driver NOTIFY driverChanged)
    Q_PROPERTY(QString device READ device NOTIFY deviceChanged)
public:
    auto input() const -> const AudioFormatObject* { return &m_input; }
    auto output() const -> const AudioFormatObject* { return &m_output; }
    auto renderer() const -> const AudioFormatObject* { return &m_renderer; }
    auto input() -> AudioFormatObject* { return &m_input; }
    auto output() -> AudioFormatObject* { return &m_output; }
    auto renderer() -> AudioFormatObject* { return &m_renderer; }
    auto normalizer() const -> double { return m_gain; }
    auto setNormalizer(double gain) -> void
        { if (_Change(m_gain, gain)) emit normalizerChanged(); }
    auto device() const -> QString;
    auto driver() const -> QString { return m_driver.toUpper(); }
public slots:
    void setDriver(const QString &driver);
    void setDevice(const QString &device);
signals:
    void normalizerChanged();
    void driverChanged();
    void deviceChanged();
private:
    AudioFormatObject m_input, m_output, m_renderer;
    double m_gain = -1.0;
    QString m_driver, m_device;
};

/******************************************************************************/

class VideoHwAccObject : public QObject {
    Q_OBJECT
    Q_PROPERTY(int state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString driver READ driver NOTIFY driverChanged)
public:
    auto state() const -> int { return m_state; }
    auto driver() const -> QString { return m_driver; }
    auto setDriver(const QString &driver) -> void
    { if (_Change(m_driver, driver.toUpper())) emit driverChanged(); }
    auto setState(int state) -> void
    { if (_Change(m_state, state)) emit stateChanged(); }
signals:
    void stateChanged();
    void driverChanged();
private:
    int m_state = 0;
    QString m_driver;
};

class VideoFormatObject : public AvCommonFormatObject {
    Q_OBJECT
    Q_PROPERTY(qreal fps READ fps NOTIFY fpsChanged)
    Q_PROPERTY(QString space READ spaceText NOTIFY spaceChanged)
    Q_PROPERTY(QString range READ rangeText NOTIFY rangeChanged)
    Q_PROPERTY(QSize size READ size NOTIFY sizeChanged)
    Q_PROPERTY(int bpp READ bpp NOTIFY bppChanged)
public:
    auto bpp() const -> int { return m_bpp; }
    auto fps() const -> qreal { return m_fps; }
    auto space() const -> ColorSpace { return m_space; }
    auto range() const -> ColorRange { return m_range; }
    auto spaceText() const -> QString;
    auto rangeText() const -> QString;
    auto setSpace(ColorSpace space) -> void
        { if (_Change(m_space, space)) emit spaceChanged(); }
    auto setRange(ColorRange range) -> void
        { if (_Change(m_range, range)) emit rangeChanged(); }
    auto size() const -> QSize { return m_size; }
    auto width() const -> int { return m_size.width(); }
    auto height() const -> int { return m_size.height(); }
    auto setWidth(int w) -> void { setSize({w, height()}); }
    auto setHeight(int h) -> void { setSize({width(), h}); }
    auto setSize(const QSize &s) -> void
        { if (_Change(m_size, s)) emit sizeChanged(m_size); }
    auto setBppSize(const QSize &s) -> void
        { if (_Change(m_bppSize, s)) updateBitrate(); }
    auto setBppSize(const QSize &s, int bpp) -> void
    {
        if (m_bppSize != s || m_bpp != bpp) {
            m_bppSize = s;
            if (_Change(m_bpp, bpp))
                emit bppChanged();
            updateBitrate();
        }
    }
    void setFps(double fps)
        { if (_Change(m_fps, fps)) { emit fpsChanged(fps); updateBitrate(); } }
signals:
    void fpsChanged(double fps);
    void spaceChanged();
    void rangeChanged();
    void bppChanged();
    void sizeChanged(const QSize &size);
private:
    auto updateBitrate() -> void
        { setBitrate(m_bpp * width() * height() * m_fps); }
    int m_bpp = 0; double m_fps = 0;
    QSize m_bppSize, m_size;
    ColorSpace m_space = ColorSpace::Auto;
    ColorRange m_range = ColorRange::Auto;
};

class VideoObject : public AvCommonObject {
    Q_OBJECT
    Q_PROPERTY(VideoFormatObject *input READ input CONSTANT FINAL)
    Q_PROPERTY(VideoFormatObject *output READ output CONSTANT FINAL)
    Q_PROPERTY(VideoFormatObject *renderer READ renderer CONSTANT FINAL)
    Q_PROPERTY(VideoHwAccObject *hwacc READ hwacc CONSTANT FINAL)
    Q_PROPERTY(int deinterlacer READ deinterlacer NOTIFY deinterlacerChanged)
    Q_PROPERTY(int droppedFrames READ droppedFrames NOTIFY droppedFramesChanged)
    Q_PROPERTY(int delayedFrames READ delayedFrames NOTIFY delayedFramesChanged)
    Q_PROPERTY(int delayedTime READ delayedTime NOTIFY delayedTimeChanged)
    Q_PROPERTY(qreal droppedFps READ droppedFps NOTIFY droppedFpsChanged)
public:
    VideoObject();
    auto input() const -> const VideoFormatObject* { return &m_input; }
    auto renderer() const -> const VideoFormatObject* { return &m_renderer; }
    auto output() const -> const VideoFormatObject* { return &m_output; }
    auto input() -> VideoFormatObject* { return &m_input; }
    auto renderer() -> VideoFormatObject* { return &m_renderer; }
    auto output() -> VideoFormatObject* { return &m_output; }
    auto hwacc() -> VideoHwAccObject* { return &m_hwacc; }
    auto hwacc() const -> const VideoHwAccObject* { return &m_hwacc; }
    auto deinterlacer() const -> int { return m_deint; }
    auto setDeinterlacer(int deint) -> void
        { if (_Change(m_deint, deint)) emit deinterlacerChanged(); }
    auto droppedFrames() const -> int { return m_dropped; }
    auto droppedFps() const -> qreal { return m_droppedFps; }
    auto delayedFrames() const -> int { return m_delayed; }
    auto setFpsManimulation(double fps) -> void
        { if (_Change(m_fpsMp, fps)) emit delayedTimeChanged(); }
    auto delayedTime() const -> int;
    void setDroppedFrames(int f);
    void setDelayedFrames(int f)
        { if (_Change(m_delayed, f)) emit delayedFramesChanged(); }
signals:
    void deinterlacerChanged();
    void droppedFramesChanged();
    void droppedFpsChanged();
    void delayedFramesChanged();
    void delayedTimeChanged();
private:
    VideoFormatObject m_input, m_output, m_renderer;
    VideoHwAccObject m_hwacc;
    int m_deint = 0, m_dropped = 0, m_delayed = 0;
    qreal m_droppedFps = 0.0, m_fpsMp = 1;
    QTime m_time;
};

/******************************************************************************/

class SubtitleObject : public AvCommonObject {
    Q_OBJECT
public:
    SubtitleObject();
};

#endif // AVINFOOBJECT_HPP
