#pragma once

#include "../includes/geode.hpp"
#include "../types/an_song.hpp"

class ANDropdownLayer : public Popup<int, Vec<Shared<ANSong>>, CustomSongWidget *, int, int> {
protected:
  int m_songID;
  Vec<Shared<ANSong>> m_songCandidates;
  int m_popupNumber;
  int m_totalPopups;
  ListView *m_listView = nullptr;
  CCSize m_cellSize = {320.f, 60.f};

  bool setup(int songId, Vec<Shared<ANSong>> songCandidates, CustomSongWidget *parent,
             int popupNumber, int totalPopups) override;

public:
  Ref<CustomSongWidget> m_parentWidget;

  static ANDropdownLayer *create(int songId, Vec<Shared<ANSong>> songCandidates,
                                 CustomSongWidget *parent, int popupNumber, int totalPopups) {
    auto ret = new ANDropdownLayer;
    if (ret && ret->initAnchored(420.f, 280.f, songId, songCandidates, parent, popupNumber,
                                 totalPopups, "GJ_square02.png")) {
      ret->autorelease();
      return ret;
    }

    CC_SAFE_DELETE(ret);
    return nullptr;
  }
};
