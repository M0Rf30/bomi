#ifndef PREF_HPP
#define PREF_HPP

#include "openmediainfo.hpp"
#include "player/mrlstate.hpp"
#include "audio/channellayoutmap.hpp"
#include "audio/audionormalizeroption.hpp"
#include "video/hwacc.hpp"
#include "video/deintcaps.hpp"
#include "video/deintoption.hpp"
#include "video/motionintrploption.hpp"
#include "misc/keymodifieractionmap.hpp"
#include "misc/osdstyle.hpp"
#include "misc/autoloader.hpp"
#include "misc/locale.hpp"
#include "enum/generateplaylist.hpp"
#include "enum/autoselectmode.hpp"
#include "enum/audiodriver.hpp"
#include "enum/clippingmethod.hpp"
#include "enum/verticalalignment.hpp"
#include "enum/quicksnapshotsave.hpp"
#include "enum/mousebehavior.hpp"
#include "quick/playlistthemeobject.hpp"
#include "quick/osdthemeobject.hpp"
#include "pref_helper.hpp"

using Shortcuts = QMap<QString, QList<QKeySequence>>;
enum class KeyMapPreset {Bomi, Movist};

class Pref : public QObject {
    Q_OBJECT
/***************************************************************/
#define P_(type, name, def, editor) \
public: \
    auto name() const -> type { return m_##name; } \
private: \
    type m_##name = def; \
    Q_PROPERTY(type name READ name WRITE set_##name) \
    auto set_##name(const type &t) { m_##name = t; } \
    Q_INVOKABLE QString editor_##name() const { return QString::fromLatin1(editor); } \
    Q_INVOKABLE bool compare_##name(const QVariant &var) const { return m_##name == var.value<type>(); } \
public:
#define P0(type, var, def) P_(type, var, def, PrefEditorProperty<type>::name)
#define P1(type, var, def, editor) P_(type, var, def, editor)
/***************************************************************/
public:
    Pref();
    P0(OpenMediaInfo, open_media_from_file_manager, {OpenMediaBehavior::NewPlaylist})
    P0(OpenMediaInfo, open_media_by_drag_and_drop, {OpenMediaBehavior::Append})
    P1(QString, quick_snapshot_format, u"png"_q, "currentText")
    P0(QString, quick_snapshot_folder, QDir::homePath())
    P0(int, quick_snapshot_quality, -1)
    P0(QuickSnapshotSave, quick_snapshot_save, QuickSnapshotSave::Fixed)

    P0(bool, fit_to_video, false)
    P0(bool, use_mpris2, true)
    P0(bool, pause_minimized, true)
    P0(bool, pause_video_only, true)
    P0(bool, remember_stopped, true)
    P0(bool, resume_ignore_in_playlist, false)
    P0(bool, precise_seeking, false)
    P0(bool, remember_image, false)
    P0(bool, enable_generate_playlist, true)
    P0(QStringList, restore_properties, defaultRestoreProperties())
    P0(GeneratePlaylist, generate_playlist, GeneratePlaylist::Folder)
    P0(bool, hide_cursor, true)
    P0(bool, disable_screensaver, true)
    P0(bool, lion_style_fullscreen, false)
    P0(bool, hide_cursor_fs_only, false)
    P0(int, hide_cursor_delay_sec, 3)
    P0(bool, show_logo, true)
    P0(QColor, bg_color, Qt::black)
    P0(bool, use_heartbeat, false)
    P0(bool, exclude_images, true)
    P0(QString, heartbeat_command, {})
    P0(int, heartbeat_interval, 60)
    P0(QStringList, sub_priority, {})
    P0(QStringList, audio_priority, {})

    P0(OsdTheme, osd_theme, defaultOsdTheme())
    P0(PlaylistTheme, playlist_theme, {})
    P0(HistoryTheme, history_theme, {})

    P0(int, blur_kern_c, 1)
    P0(int, blur_kern_n, 2)
    P0(int, blur_kern_d, 1)
    P0(int, sharpen_kern_c, 5)
    P0(int, sharpen_kern_n, -1)
    P0(int, sharpen_kern_d, 0)
    P0(MotionIntrplOption, motion_interpolation, {})

    P0(ChannelLayoutMap, channel_manipulation, ChannelLayoutMap::default_())

    P0(Autoloader, sub_autoload_v2, defaultSubtitleAutoload())
    P0(Autoloader, audio_autoload, defaultAutioAutoload())
    P0(bool, sub_enable_autoselect, true)
    P0(bool, sub_enc_autodetection, true)
    P0(AutoselectMode, sub_autoselect, AutoselectMode::Matched)
    P1(QString, sub_enc, defaultSubtitleEncoding(), "encoding")
    P1(QString, sub_ext, {}, "value")
    P0(int, sub_enc_accuracy, defaultSubtitleEncodingDetectionAccuracy())
    P0(int, ms_per_char, 500)
    P0(OsdStyle, sub_style, {})

    P0(bool, enable_system_tray, true)
    P0(bool, hide_rather_close, true)
    P0(MouseActionMap, mouse_action_map, defaultMouseActionMap())
    P0(bool, invert_wheel, false)
    P0(int, seek_step1_sec, 5)
    P0(int, seek_step2_sec, 30)
    P0(int, seek_step3_sec, 60)

    P0(int, speed_step, 10)
    P0(int, brightness_step, 1)
    P0(int, saturation_step, 1)
    P0(int, contrast_step, 1)
    P0(int, hue_step, 1)
    P0(int, volume_step, 2)
    P0(double, sub_sync_step_sec, 0.5)
    P0(double, audio_sync_step_sec, 0.2)
    P0(int, amp_step, 10)
    P0(int, sub_pos_step, 1)
    P0(bool, enable_hwaccel, false)
    P0(QStringList, hwaccel_codecs, defaultHwAccCodecs())
    P0(DeintOptionSet, deinterlacing, {})

    P0(AudioNormalizerOption, audio_normalizer, AudioNormalizerOption::default_())

    P1(QString, skin_name, defaultSkinName(), "currentText")

    P0(Shortcuts, shortcuts, defaultShortcuts())

    P1(QString, audio_device, u"auto"_q, "currentText")
    P0(ClippingMethod, clipping_method, ClippingMethod::Auto)

    P0(int, cache_local, 0)
    P0(int, cache_network, 25000)
    P0(int, cache_disc, 0)
    P0(int, cache_min_playback, 0)
    P0(int, cache_min_seeking, 2)
    P0(QStringList, network_folders, {})

    P0(QString, yt_user_agent, u"Mozilla/5.0 (X11; Linux x86_64; rv:10.0) Gecko/20100101 Firefox/10.0 (Chrome)"_q)
    P0(QString, yt_program, u"youtube-dl"_q)
    P0(QString, yle_program, u"yle-dl"_q)
    int yt_timeout = 60000;

    QStringList sub_search_paths;

    P0(bool, app_unique, true);
    P1(Locale, app_locale, {}, "locale");
    P1(QString, app_style, {}, "currentText");

    static auto preset(KeyMapPreset id) -> Shortcuts;

    auto save() const -> void;
    auto load() -> void;

    auto initialize() -> void;
private:
    static auto defaultSubtitleAutoload() -> Autoloader;
    static auto defaultAutioAutoload() -> Autoloader;
    static auto defaultHwAccBackend() -> HwAcc::Type;
    static auto defaultRestoreProperties() -> QStringList;
    static auto defaultOsdTheme() -> OsdTheme;
    static auto defaultSkinName() -> QString;
    static auto defaultSubtitleEncoding() -> QString;
    static auto defaultSubtitleEncodingDetectionAccuracy() -> int;
    static auto defaultHwAccCodecs() -> QStringList;
    static auto defaultHwAccDeints() -> QVector<DeintMethod>;
    static auto defaultShortcuts() -> Shortcuts;
    static auto defaultMouseActionMap() -> MouseActionMap;
};
#undef P_
#undef P0
#undef P1
#undef P2

Q_DECLARE_METATYPE(QVector<QMetaProperty>);
Q_DECLARE_METATYPE(QList<MatchString>);
Q_DECLARE_METATYPE(Shortcuts);

#endif // PREF_HPP
