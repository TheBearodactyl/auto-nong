#include "an_song_cell.hpp"
#include "../types/an_song.hpp"
#include "../utils/utils.hpp"
#include "Geode/ui/Notification.hpp"
#include "Geode/utils/cocos.hpp"
#include "fleym.nongd/include/jukebox.hpp"
#include "fleym.nongd/src/types/song_info.hpp"
#include "list_cell.hpp"
#include <filesystem>

bool ANSongCell::init(int songId, ANSong *song, ANDropdownLayer *parentPopup, CCSize const &size) {
  if (!JBListCell::init(size))
    return false;

  m_songId = songId;
  m_anSong = song;
  m_parentPopup = parentPopup;

  CCMenuItemSpriteExtra *button;
  CCSprite *spr;
  CCMenu *menu;

  // Download button
  spr = CCSprite::createWithSpriteFrameName("GJ_downloadBtn_001.png");
  spr->setScale(0.7f);
  button = CCMenuItemSpriteExtra::create(spr, this, menu_selector(ANSongCell::onDownload));
  button->setAnchorPoint(ccp(0.5f, 0.5f));
  button->setID("set-button");

  menu = CCMenu::create();
  menu->addChild(button);
  menu->setAnchorPoint(ccp(0, 0));
  menu->setPosition(ccp(295.f, 18.f));
  menu->setID("button-menu1");

  this->addChild(menu);

  // Set button
  spr = ButtonSprite::create("Set");
  spr->setScale(0.6f);

  button = CCMenuItemSpriteExtra::create(spr, this, menu_selector(ANSongCell::onSetSong));

  menu = CCMenu::create();
  menu->addChild(button);
  menu->setAnchorPoint(ccp(0, 0));
  menu->setPosition(ccp(290.f, 42.f));
  menu->setID("button-menu2");

  this->addChild(menu);

  // Delete button
  spr = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
  spr->setScale(0.65f);
  button = CCMenuItemSpriteExtra::create(spr, this, menu_selector(ANSongCell::onDeleteSong));
  menu = CCMenu::create();
  menu->addChild(button);

  menu->setAnchorPoint(ccp(0, 0));
  menu->setPosition(ccp(265.f, 18.f));
  menu->setID("button-menu2");
  this->addChild(menu);
  m_trashButton = button;

  setButtonsVisible();

  m_songInfoLayer = CCLayer::create();
  m_songNameLabel = CCLabelBMFont::create(m_anSong->m_name.c_str(), "bigFont.fnt");
  m_songNameLabel->limitLabelWidth(220.f, 0.7f, 0.1f);
  m_songNameLabel->setID("song-name");
  m_artistNameLabel = CCLabelBMFont::create(m_anSong->m_artist.c_str(), "goldFont.fnt");
  m_artistNameLabel->limitLabelWidth(120.f, 0.7f, 0.1f);
  m_artistNameLabel->setID("artist-name");

  m_sourceLabel = CCLabelBMFont::create(
      fmt::format("{} : {}", getSubstringAfterSlash(m_anSong->m_index), m_anSong->getSource())
          .c_str(),
      "goldFont.fnt");
  m_sourceLabel->limitLabelWidth(120.f, 0.7f, 0.1f);
  m_sourceLabel->setID("artist-name");

  m_songInfoLayer->addChild(m_sourceLabel);
  m_songInfoLayer->addChild(m_artistNameLabel);
  m_songInfoLayer->addChild(m_songNameLabel);
  m_songInfoLayer->setID("song-info");
  auto layout = ColumnLayout::create();
  layout->setAutoScale(false);
  layout->setAxisAlignment(AxisAlignment::Center);
  layout->setCrossAxisLineAlignment(AxisAlignment::Start);
  m_songInfoLayer->setContentSize(ccp(240.f, this->getContentSize().height - 6.f));
  m_songInfoLayer->setAnchorPoint(ccp(0.f, 0.f));
  m_songInfoLayer->setPosition(ccp(12.f, 1.5f));
  m_songInfoLayer->setLayout(layout);

  this->addChild(m_songInfoLayer);
  return true;
}

void ANSongCell::onSetSong(CCObject *target) { setSong(); }

void ANSongCell::setButtonsVisible() {
  m_trashButton->setVisible(fs::exists(getFileDownloadPath(false)));
}

void ANSongCell::onDeleteSong(CCObject *target) {
  const fs::path downloadPath = getFileDownloadPath(false);
  if (!fs::exists(downloadPath)) {
    Notification::create("File doesn't exist", NotificationIcon::Error)->show();
    return;
  }
  fs::remove(downloadPath);
  setButtonsVisible();
}

void ANSongCell::setSong() {
  if (m_parentPopup->m_uID != m_parentPopupUID) {
    return;
  }

  setButtonsVisible();

  fs::path download_path = getFileDownloadPath(false);
  const Ref<CustomSongWidget> custom_song_widget = m_parentPopup->m_parentWidget;

  if (!fs::exists(download_path)) {
    Notification::create("File doesn't exist", NotificationIcon::Error)->show();
    return;
  }

  jukebox::SongInfo song = {
      .path = download_path,
      .songName = m_anSong->m_name,
      .authorName = m_anSong->m_artist,
      .songUrl = "local",
  };

  jukebox::addNong(song, m_songId);
  jukebox::setActiveSong(song, m_songId);

  custom_song_widget->m_songInfoObject->m_artistName = m_anSong->m_artist;
  custom_song_widget->m_songInfoObject->m_songName = m_anSong->m_name;
  custom_song_widget->updateSongObject(custom_song_widget->m_songInfoObject);
}

fs::path ANSongCell::getFileDownloadPath(bool create) {
#ifdef GEODE_IS_ANDROID
  String baseDir = "/storage/emulated/0/nongs";
  if (!fs::exists(baseDir)) {
    fs::create_directories(baseDir);
  }
#else
  String baseDir = Mod::get()->getSaveDir().string();
#endif

  if (typeid(*m_anSong) == typeid(ANYTSong)) {
    const ANYTSong *ytSong = static_cast<ANYTSong *>(m_anSong);
    const String videoId = ytSong->m_ytId;
    if (create) {
      fs::create_directories(fmt::format("{}/youtube", baseDir));
    }
    return fmt::format("{}/youtube/{}.mp3", baseDir, videoId);
  }
  if (typeid(*m_anSong) == typeid(ANHostSong)) {
    const ANHostSong *hostSong = static_cast<ANHostSong *>(m_anSong);
    if (create) {
      fs::create_directories(fmt::format("{}/host", baseDir));
    }
    return fmt::format("{}/host/{}.mp3", baseDir, urlToFilename(hostSong->m_url));
  }
  return "";
}

void ANSongCell::downloadFromYtDlp() {
  m_currentlyDownloading = true;
  const ANYTSong *ytSong = static_cast<ANYTSong *>(m_anSong);
  const fs::path ytDlpPath = Mod::get()->getSettingValue<String>("yt-dlp-path");
  const String videoId = ytSong->m_ytId;
  const fs::path downloadPath = getFileDownloadPath(true);

  if (!fs::exists(ytDlpPath)) {
    log::info("yt-dlp.exe not found at specified path: {}", ytDlpPath);
    Notification::create("yt-dlp.exe not found at specified path", NotificationIcon::Error)->show();
    return;
  }

  String ytDlpCommand = fmt::format("\"{}\" -x --audio-format mp3 \"{}\" -o \"{}\"", ytDlpPath,
                                    videoId, downloadPath);

  auto notif = Notification::create("Downloading using yt dlp", NotificationIcon::Loading, 0);
  notif->setTime(NOTIFICATION_DEFAULT_TIME);
  notif->show();

  std::thread commandThread([this, ytSong, ytDlpCommand]() {
    String command = ytDlpCommand;

    int result = std::system(fmt::format("\"{}\"", command).c_str());

    log::info("command({}): {}", result, command);

    m_currentlyDownloading = false;

    if (result != 0) {
      log::info("Failed to execute command({}): {}", result, command);
      Loader::get()->queueInMainThread([]() {
        Notification::create("Failed to download from YouTube", NotificationIcon::Error)->show();
      });
      return;
    }

    log::info("Successfully executed command: {}", command);

    Loader::get()->queueInMainThread([this]() {
      Notification::create("Downloaded", NotificationIcon::Success)->show();
      setSong();
    });
  });

  commandThread.detach();
}

void ANSongCell::downloadFromCobalt() {
  m_currentlyDownloading = true;
  const String apiUrl = "https://co.wuk.sh/api/json";
  const ANYTSong *ytSong = static_cast<ANYTSong *>(m_anSong);
  const String videoId = ytSong->m_ytId;

  std::unordered_map<String, String> parameters = {
      {"url", fmt::format("https://www.youtube.com/watch?v={}", videoId)},
      {"aFormat", "mp3"},
      {"isAudioOnly", "true"}};

  matjson::Value parameters_json = matjson::Object();
  for (auto const &[key, value] : parameters) {
    parameters_json[key] = value;
  }
  String body = parameters_json.dump(matjson::NO_INDENTATION);

  auto notif = Notification::create("Downloading using Cobalt Tools", NotificationIcon::Loading, 0);
  notif->setTime(NOTIFICATION_DEFAULT_TIME);
  notif->show();

  geode::utils::web::AsyncWebRequest()
      .header("Accept", "application/json")
      .header("Content-Type", "application/json")
      .bodyRaw(body)
      .post(apiUrl)
      .text()
      .then([this](const String &r) {
        matjson::Value data = matjson::parse(r);

        if (data["status"] == "stream") {
          String audio_url = data["url"].as_string();

          geode::utils::web::AsyncWebRequest()
              .get(audio_url)
              .bytes()
              .then([this](const geode::ByteVector &audio_data) {
                std::ofstream file(getFileDownloadPath(true), std::ios::out | std::ios::binary);
                file.write(reinterpret_cast<const char *>(audio_data.data()), audio_data.size());
                file.close();

                log::info("Downloaded the song successfully!");

                Loader::get()->queueInMainThread([this]() {
                  Notification::create("Downloaded", NotificationIcon::Success)->show();
                  setSong();
                });
                m_currentlyDownloading = false;
              })
              .expect([this](auto _) { m_currentlyDownloading = false; });
        } else {
          log::warn("Failed to download the song: {}", r);
          m_currentlyDownloading = false;
        }
      })
      .expect([this](auto _) { m_currentlyDownloading = false; });
}

void ANSongCell::onDownload(CCObject *target) {
  if (m_currentlyDownloading) {
    FLAlertLayer::create(
        "Currently downloading",
        "Please wait for the current download to finish before downloading the same song again.",
        "OK")
        ->show();
    return;
  }

  if (fs::exists(getFileDownloadPath(false))) {
    createQuickPopup("Song already downloaded", "Are you sure you want to redownload the song?",
                     "No", "Yes", [this, target](auto, bool btn2) {
                       if (!btn2) {
                         return;
                       }
                       fs::remove(getFileDownloadPath(false));
                       onDownload(target);
                     });
    return;
  }

  if (typeid(*m_anSong) == typeid(ANYTSong)) {
    if (Mod::get()->getSettingValue<bool>("use-cobalt")) {
      downloadFromCobalt();
    } else if (Mod::get()->getSettingValue<bool>("use-yt-dlp")) {
      downloadFromYtDlp();
    } else {
      Notification::create("No download method for YouTube", NotificationIcon::Error)->show();
    }
  } else if (typeid(*m_anSong) == typeid(ANHostSong)) {
    const ANHostSong *hostSong = static_cast<ANHostSong *>(m_anSong);
    const fs::path downloadPath = getFileDownloadPath(true);
    m_currentlyDownloading = true;
    Notification::create("Downloading from URL", NotificationIcon::Loading)->show();
    geode::utils::web::AsyncWebRequest()
        .get(hostSong->m_url)
        .bytes()
        .then([this, downloadPath](const geode::ByteVector &audio_data) {
          std::ofstream file(downloadPath, std::ios::out | std::ios::binary);
          file.write(reinterpret_cast<const char *>(audio_data.data()), audio_data.size());
          file.close();

          log::info("Downloaded the song successfully!");

          Loader::get()->queueInMainThread([this]() {
            Notification::create("Downloaded", NotificationIcon::Success)->show();
            setSong();
          });
          m_currentlyDownloading = false;
        })
        .expect([this](auto _) { m_currentlyDownloading = false; });
  }
}

ANSongCell *ANSongCell::create(int songId, ANSong *song, ANDropdownLayer *parentPopup,
                               CCSize const &size) {
  auto ret = new ANSongCell();
  if (ret && ret->init(songId, song, parentPopup, size)) {
    return ret;
  }
  CC_SAFE_DELETE(ret);
  return nullptr;
}
