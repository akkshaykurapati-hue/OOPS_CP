/**
 * Smart Playlist Synthesizer — Modern Streaming Audio Client
 * Real HTML5 Audio Playback Engine, Drag-and-Drop Music Uploader & C++17 REST API Integration
 */

(function () {
  'use strict';

  // ── Application State ────────────────────────────────────────────────
  const state = {
    songs: [],
    playlists: [],
    selectedPlaylistId: null,
    selectedPlaylistDetails: null,

    // Real Audio Playback State
    currentSong: null,
    currentQueue: [],
    currentQueueIndex: -1,
    activePlaylistId: null,
    isPlaying: false,
    volume: 0.8,
    isMuted: false,
    repeatMode: 'off', // 'off' | 'all' | 'one'
    isShuffle: false,
    shuffleHistory: [],
    isSeeking: false,

    // Navigation & UI filters
    activeView: 'home',
    genreFilter: 'all',
    searchQuery: '',
    targetSongForPlaylist: null,
    selectedUploadFile: null,
    backendOnline: true
  };

  const API_BASE = ''; // Relative path because backend serves web/

  // ── Native HTML5 Audio Engine (Source of truth) ──────────────────────
  const audio = document.getElementById('realAudio');
  audio.volume = state.volume;

  // ── DOM Element Cache ────────────────────────────────────────────────
  const elements = {
    // Navigation
    navItems: document.querySelectorAll('.nav-item'),
    viewPanels: document.querySelectorAll('.view-panel'),
    sidebarPlaylistsList: document.getElementById('sidebarPlaylistsList'),
    backendBadge: document.getElementById('backendBadge'),

    // Loading & Error states
    mainLoadingState: document.getElementById('mainLoadingState'),
    mainLoadingText: document.getElementById('mainLoadingText'),
    mainErrorBanner: document.getElementById('mainErrorBanner'),
    mainErrorTitle: document.getElementById('mainErrorTitle'),
    mainErrorDesc: document.getElementById('mainErrorDesc'),
    btnRetryConnection: document.getElementById('btnRetryConnection'),

    // Topbar & Actions
    globalSearchInput: document.getElementById('globalSearchInput'),
    searchClearBtn: document.getElementById('searchClearBtn'),
    btnRefreshAll: document.getElementById('btnRefreshAll'),
    btnOpenUploadModal: document.getElementById('btnOpenUploadModal'),
    btnOpenAddSongModal: document.getElementById('btnOpenAddSongModal'),
    btnShortcutsInfo: document.getElementById('btnShortcutsInfo'),
    btnOpenCreatePlaylistModal: document.getElementById('btnOpenCreatePlaylistModal'),
    btnOpenLoadPlaylistModal: document.getElementById('btnOpenLoadPlaylistModal'),

    // Home View
    statSongsCount: document.getElementById('statSongsCount'),
    statPlaylistsCount: document.getElementById('statPlaylistsCount'),
    statTotalDuration: document.getElementById('statTotalDuration'),
    heroPlayActiveBtn: document.getElementById('heroPlayActiveBtn'),
    heroExploreLibraryBtn: document.getElementById('heroExploreLibraryBtn'),
    homePlaylistsShelf: document.getElementById('homePlaylistsShelf'),
    homeFeaturedTracksGrid: document.getElementById('homeFeaturedTracksGrid'),
    linkViewAllPlaylists: document.getElementById('linkViewAllPlaylists'),
    linkViewAllTracks: document.getElementById('linkViewAllTracks'),

    // Library View
    genrePillsContainer: document.getElementById('genrePillsContainer'),
    librarySortSelect: document.getElementById('librarySortSelect'),
    btnLibraryUpload: document.getElementById('btnLibraryUpload'),
    masterSongTable: document.getElementById('masterSongTable'),
    masterSongTableBody: document.getElementById('masterSongTableBody'),
    libraryEmptyState: document.getElementById('libraryEmptyState'),
    btnEmptyUploadMusic: document.getElementById('btnEmptyUploadMusic'),

    // Playlists View
    allPlaylistsGrid: document.getElementById('allPlaylistsGrid'),
    btnCreatePlaylistHeader: document.getElementById('btnCreatePlaylistHeader'),
    playlistDetailContainer: document.getElementById('playlistDetailContainer'),
    detailArtwork: document.getElementById('detailArtwork'),
    detailTitle: document.getElementById('detailTitle'),
    detailStats: document.getElementById('detailStats'),
    btnDetailPlay: document.getElementById('btnDetailPlay'),
    btnDetailShuffle: document.getElementById('btnDetailShuffle'),
    btnDetailSort: document.getElementById('btnDetailSort'),
    btnDetailSave: document.getElementById('btnDetailSave'),
    btnDetailRename: document.getElementById('btnDetailRename'),
    btnDetailDelete: document.getElementById('btnDetailDelete'),
    playlistTracksTable: document.getElementById('playlistTracksTable'),
    playlistTracksTableBody: document.getElementById('playlistTracksTableBody'),
    playlistTracksEmptyState: document.getElementById('playlistTracksEmptyState'),
    btnEmptyAddFromLibrary: document.getElementById('btnEmptyAddFromLibrary'),

    // Search View
    searchResultsHeading: document.getElementById('searchResultsHeading'),
    searchResultsSubtitle: document.getElementById('searchResultsSubtitle'),
    searchTable: document.getElementById('searchTable'),
    searchTableBody: document.getElementById('searchTableBody'),
    searchEmptyState: document.getElementById('searchEmptyState'),

    // Bottom Player Controls
    playerArtwork: document.getElementById('playerArtwork'),
    playerWaveAnim: document.getElementById('playerWaveAnim'),
    playerBufferSpinner: document.getElementById('playerBufferSpinner'),
    playerTrackTitle: document.getElementById('playerTrackTitle'),
    playerTrackArtist: document.getElementById('playerTrackArtist'),
    playerTrackAlbum: document.getElementById('playerTrackAlbum'),
    trackAlbumSep: document.getElementById('trackAlbumSep'),
    playerFormatBadge: document.getElementById('playerFormatBadge'),
    playerActivePlName: document.getElementById('playerActivePlName'),
    playerElapsed: document.getElementById('playerElapsed'),
    playerDuration: document.getElementById('playerDuration'),
    bufferFill: document.getElementById('bufferFill'),
    progressFill: document.getElementById('progressFill'),
    progressSlider: document.getElementById('progressSlider'),
    playerShuffleBtn: document.getElementById('playerShuffleBtn'),
    playerPrevBtn: document.getElementById('playerPrevBtn'),
    playerPlayPauseBtn: document.getElementById('playerPlayPauseBtn'),
    iconPlay: document.getElementById('iconPlay'),
    iconPause: document.getElementById('iconPause'),
    playerNextBtn: document.getElementById('playerNextBtn'),
    playerRepeatBtn: document.getElementById('playerRepeatBtn'),
    repeatModeBadge: document.getElementById('repeatModeBadge'),
    playerMuteBtn: document.getElementById('playerMuteBtn'),
    iconVolHigh: document.getElementById('iconVolHigh'),
    iconVolMuted: document.getElementById('iconVolMuted'),
    playerVolumeSlider: document.getElementById('playerVolumeSlider'),
    playerVolumeValue: document.getElementById('playerVolumeValue'),

    // Modals
    modalUploadMusic: document.getElementById('modalUploadMusic'),
    formUploadMusic: document.getElementById('formUploadMusic'),
    uploadDropZone: document.getElementById('uploadDropZone'),
    uploadAudioInput: document.getElementById('uploadAudioInput'),
    uploadFileMeta: document.getElementById('uploadFileMeta'),
    uploadDetectedFormat: document.getElementById('uploadDetectedFormat'),
    uploadSelectedFilename: document.getElementById('uploadSelectedFilename'),
    uploadSelectedFilesize: document.getElementById('uploadSelectedFilesize'),
    btnClearUploadedFile: document.getElementById('btnClearUploadedFile'),
    uploadTitle: document.getElementById('uploadTitle'),
    uploadArtist: document.getElementById('uploadArtist'),
    uploadAlbum: document.getElementById('uploadAlbum'),
    uploadGenre: document.getElementById('uploadGenre'),
    uploadDuration: document.getElementById('uploadDuration'),
    uploadArtwork: document.getElementById('uploadArtwork'),
    uploadProgressWrapper: document.getElementById('uploadProgressWrapper'),
    uploadProgressBar: document.getElementById('uploadProgressBar'),
    uploadProgressPercent: document.getElementById('uploadProgressPercent'),
    uploadStatusText: document.getElementById('uploadStatusText'),
    uploadErrorAlert: document.getElementById('uploadErrorAlert'),
    btnSubmitUpload: document.getElementById('btnSubmitUpload'),

    modalEditSong: document.getElementById('modalEditSong'),
    formEditSong: document.getElementById('formEditSong'),
    editSongId: document.getElementById('editSongId'),
    editSongTitle: document.getElementById('editSongTitle'),
    editSongArtist: document.getElementById('editSongArtist'),
    editSongAlbum: document.getElementById('editSongAlbum'),
    editSongGenre: document.getElementById('editSongGenre'),
    editSongDuration: document.getElementById('editSongDuration'),
    editSongArtwork: document.getElementById('editSongArtwork'),
    editSongSubtitle: document.getElementById('editSongSubtitle'),

    modalShortcuts: document.getElementById('modalShortcuts'),
    modalCreatePlaylist: document.getElementById('modalCreatePlaylist'),
    formCreatePlaylist: document.getElementById('formCreatePlaylist'),
    inputPlaylistName: document.getElementById('inputPlaylistName'),
    modalAddSong: document.getElementById('modalAddSong'),
    formAddSong: document.getElementById('formAddSong'),
    modalAddToPlaylist: document.getElementById('modalAddToPlaylist'),
    pickerSongPreview: document.getElementById('pickerSongPreview'),
    selectTargetPlaylist: document.getElementById('selectTargetPlaylist'),
    btnConfirmAddToPlaylist: document.getElementById('btnConfirmAddToPlaylist'),
    modalLoadPlaylist: document.getElementById('modalLoadPlaylist'),
    formLoadPlaylist: document.getElementById('formLoadPlaylist'),
    inputLoadFilename: document.getElementById('inputLoadFilename'),

    toastContainer: document.getElementById('toastContainer')
  };

  // ── Main Content Loading & Error States ──────────────────────────────
  function showLoading(msg = 'Connecting to C++17 Synthesizer Engine...') {
    if (elements.mainLoadingState) {
      if (elements.mainLoadingText) elements.mainLoadingText.textContent = msg;
      elements.mainLoadingState.style.display = 'flex';
    }
  }

  function hideLoading() {
    if (elements.mainLoadingState) {
      elements.mainLoadingState.style.display = 'none';
    }
  }

  function showMainError(title, desc) {
    if (elements.mainErrorBanner) {
      if (elements.mainErrorTitle) elements.mainErrorTitle.textContent = title;
      if (elements.mainErrorDesc) elements.mainErrorDesc.textContent = desc;
      elements.mainErrorBanner.style.display = 'flex';
    }
  }

  function hideMainError() {
    if (elements.mainErrorBanner) {
      elements.mainErrorBanner.style.display = 'none';
    }
  }

  // ── Network API Helper ───────────────────────────────────────────────
  async function apiFetch(endpoint, options = {}) {
    try {
      const res = await fetch(API_BASE + endpoint, {
        headers: { 'Content-Type': 'application/json', ...options.headers },
        ...options
      });

      const data = await res.json().catch(() => ({}));

      if (!res.ok) {
        throw new Error(data.error || `HTTP ${res.status}: ${res.statusText}`);
      }

      setBackendStatus(true);
      return data;
    } catch (err) {
      if (err.message.includes('Failed to fetch') || err.message.includes('NetworkError')) {
        setBackendStatus(false);
      }
      throw err;
    }
  }

  function setBackendStatus(online) {
    if (state.backendOnline === online) return;
    state.backendOnline = online;
    if (online) {
      elements.backendBadge.innerHTML = '<span class="status-dot online"></span><span class="status-label">C++17 Backend: Online</span>';
    } else {
      elements.backendBadge.innerHTML = '<span class="status-dot offline"></span><span class="status-label">C++17 Backend: Offline</span>';
    }
  }

  // ── Toast Notification System ────────────────────────────────────────
  function showToast(message, type = 'success') {
    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;
    
    let icon = '✓';
    if (type === 'error') icon = '✕';
    else if (type === 'warning') icon = '⚠';

    toast.innerHTML = `<span class="toast-icon">${icon}</span><span>${escapeHtml(message)}</span>`;
    elements.toastContainer.appendChild(toast);

    setTimeout(() => {
      toast.style.opacity = '0';
      toast.style.transform = 'translateY(12px)';
      toast.style.transition = 'all 0.3s ease';
      setTimeout(() => toast.remove(), 300);
    }, 4000);
  }

  function escapeHtml(str) {
    if (!str) return '';
    return String(str)
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;')
      .replace(/'/g, '&#039;');
  }

  function formatDuration(seconds) {
    if (isNaN(seconds) || seconds < 0) return '0:00';
    const s = Math.round(seconds);
    const m = Math.floor(s / 60);
    const rem = s % 60;
    return `${m}:${rem < 10 ? '0' : ''}${rem}`;
  }

  function formatBytes(bytes) {
    if (bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
  }

  // ── Real Audio Player Engine ─────────────────────────────────────────

  function setupAudioEngine() {
    // 1. Play Event
    audio.addEventListener('play', () => {
      state.isPlaying = true;
      elements.iconPlay.style.display = 'none';
      elements.iconPause.style.display = 'block';
      elements.playerWaveAnim.style.display = 'flex';
      updateActiveRowHighlight();
    });

    // 2. Pause Event
    audio.addEventListener('pause', () => {
      state.isPlaying = false;
      elements.iconPlay.style.display = 'block';
      elements.iconPause.style.display = 'none';
      elements.playerWaveAnim.style.display = 'none';
      updateActiveRowHighlight();
    });

    // 3. Time Update Event (Source of truth for playback position)
    audio.addEventListener('timeupdate', () => {
      if (!state.isSeeking && audio.duration) {
        const current = audio.currentTime;
        const total = audio.duration;
        const percent = (current / total) * 100;

        elements.playerElapsed.textContent = formatDuration(current);
        elements.progressFill.style.width = `${percent}%`;
        elements.progressSlider.value = percent;
      }
    });

    // 4. Metadata Loaded
    audio.addEventListener('loadedmetadata', () => {
      elements.playerDuration.textContent = formatDuration(audio.duration);
    });

    // 5. Buffer Progress Event
    audio.addEventListener('progress', () => {
      if (audio.buffered.length > 0 && audio.duration > 0) {
        const bufferedEnd = audio.buffered.end(audio.buffered.length - 1);
        const percent = (bufferedEnd / audio.duration) * 100;
        elements.bufferFill.style.width = `${Math.min(100, percent)}%`;
      }
    });

    // 6. Buffering / Waiting state
    audio.addEventListener('waiting', () => {
      elements.playerBufferSpinner.style.display = 'block';
    });
    audio.addEventListener('canplay', () => {
      elements.playerBufferSpinner.style.display = 'none';
    });
    audio.addEventListener('playing', () => {
      elements.playerBufferSpinner.style.display = 'none';
    });

    // 7. Track Ended Event: Auto-Advance & Repeat handling
    audio.addEventListener('ended', () => {
      if (state.repeatMode === 'one') {
        audio.currentTime = 0;
        audio.play().catch(console.error);
        return;
      }

      // Normal Queue Advance
      if (state.currentQueue.length > 0) {
        if (state.isShuffle) {
          playNextShuffleTrack();
        } else if (state.currentQueueIndex < state.currentQueue.length - 1) {
          playQueueIndex(state.currentQueueIndex + 1);
        } else if (state.repeatMode === 'all') {
          playQueueIndex(0); // Loop back to start
        } else {
          // Reached end of queue without repeat
          state.isPlaying = false;
          elements.iconPlay.style.display = 'block';
          elements.iconPause.style.display = 'none';
          elements.playerWaveAnim.style.display = 'none';
          updateActiveRowHighlight();
        }
      }
    });

    // 8. Playback Error Handling
    audio.addEventListener('error', (e) => {
      elements.playerBufferSpinner.style.display = 'none';
      state.isPlaying = false;
      elements.iconPlay.style.display = 'block';
      elements.iconPause.style.display = 'none';
      elements.playerWaveAnim.style.display = 'none';

      const err = audio.error;
      let msg = 'Audio playback error.';
      if (err) {
        if (err.code === MediaError.MEDIA_ERR_SRC_NOT_SUPPORTED) {
          msg = 'Audio format not supported or media file not found on server.';
        } else if (err.code === MediaError.MEDIA_ERR_NETWORK) {
          msg = 'Network connection lost while streaming audio.';
        } else if (err.code === MediaError.MEDIA_ERR_DECODE) {
          msg = 'Corrupted audio stream could not be decoded.';
        }
      }
      showToast(msg, 'error');
    });
  }

  // ── Audio Controls & Navigation ──────────────────────────────────────

  async function playSong(song, queue = null, index = -1, playlistId = null) {
    if (!song) return;

    if (queue && queue.length > 0) {
      state.currentQueue = queue;
      state.currentQueueIndex = (index >= 0) ? index : queue.findIndex(s => s.id === song.id);
    } else if (state.currentQueue.length === 0 || !state.currentQueue.some(s => s.id === song.id)) {
      state.currentQueue = state.songs;
      state.currentQueueIndex = state.songs.findIndex(s => s.id === song.id);
    }

    state.activePlaylistId = playlistId;
    state.currentSong = song;

    // Update bottom player UI
    elements.playerTrackTitle.textContent = song.title;
    elements.playerTrackArtist.textContent = song.artist;
    if (song.album && song.album !== 'Single') {
      elements.playerTrackAlbum.textContent = song.album;
      elements.playerTrackAlbum.style.display = 'inline';
      elements.trackAlbumSep.style.display = 'inline';
    } else {
      elements.playerTrackAlbum.style.display = 'none';
      elements.trackAlbumSep.style.display = 'none';
    }

    // Artwork update
    if (song.artworkUrl) {
      elements.playerArtwork.innerHTML = `<img src="${escapeHtml(song.artworkUrl)}" alt="Cover" style="width:100%;height:100%;object-fit:cover;border-radius:4px;">`;
    } else {
      elements.playerArtwork.innerHTML = `
        <svg viewBox="0 0 24 24" width="22" height="22" fill="currentColor">
          <path d="M12 3v10.55c-.59-.34-1.27-.55-2-.55-2.21 0-4 1.79-4 4s1.79 4 4 4 4-1.79 4-4V7h4V3h-6z"/>
        </svg>
        <div class="player-bars-wave" id="playerWaveAnim" style="display:none;">
          <span></span><span></span><span></span><span></span>
        </div>
      `;
      elements.playerWaveAnim = document.getElementById('playerWaveAnim');
    }

    // Format badge
    let fmt = 'AUDIO';
    if (song.audioUrl) {
      const ext = song.audioUrl.split('.').pop().toLowerCase();
      if (['mp3', 'wav', 'ogg', 'flac'].includes(ext)) {
        fmt = ext.toUpperCase();
      } else {
        fmt = 'REAL AUDIO';
      }
    }
    elements.playerFormatBadge.textContent = fmt;

    // Active playlist tag
    if (elements.playerActivePlName) {
      if (playlistId !== null) {
        const pl = state.playlists.find(p => p.id === playlistId);
        elements.playerActivePlName.textContent = pl ? pl.name : 'Playlist';
      } else {
        elements.playerActivePlName.textContent = 'Master Library';
      }
    }

    // Load real audio endpoint
    const streamUrl = `/api/songs/${song.id}/audio`;
    audio.src = streamUrl;
    elements.playerElapsed.textContent = '0:00';
    elements.playerDuration.textContent = song.formattedDuration || '0:00';
    elements.progressFill.style.width = '0%';
    elements.bufferFill.style.width = '0%';
    elements.progressSlider.value = 0;

    try {
      await audio.play();
      showToast(`Playing "${song.title}" by ${song.artist}`, 'success');

      // Inform C++ backend player to synchronize internal domain state
      if (playlistId !== null) {
        apiFetch('/api/player/play', {
          method: 'POST',
          body: JSON.stringify({ playlistId, trackIndex: state.currentQueueIndex })
        }).catch(() => {});
      }
    } catch (err) {
      console.warn('Playback initiation error:', err);
      state.isPlaying = false;
      elements.iconPlay.style.display = 'block';
      elements.iconPause.style.display = 'none';
      if (elements.playerWaveAnim) elements.playerWaveAnim.style.display = 'none';
      updateActiveRowHighlight();

      if (err.name === 'NotAllowedError') {
        showToast('Autoplay blocked by browser. Click play to start listening.', 'warning');
      } else {
        showToast(`Could not play audio: ${err.message || 'Media stream error'}`, 'error');
      }
    }

    updateActiveRowHighlight();
  }

  function playQueueIndex(index) {
    if (index >= 0 && index < state.currentQueue.length) {
      state.currentQueueIndex = index;
      playSong(state.currentQueue[index], state.currentQueue, index, state.activePlaylistId);
    }
  }

  function playNextShuffleTrack() {
    if (state.currentQueue.length <= 1) {
      playQueueIndex(0);
      return;
    }

    // Avoid repeating recent tracks in shuffle
    const available = [];
    for (let i = 0; i < state.currentQueue.length; ++i) {
      if (i !== state.currentQueueIndex && !state.shuffleHistory.includes(i)) {
        available.push(i);
      }
    }

    let nextIdx;
    if (available.length > 0) {
      nextIdx = available[Math.floor(Math.random() * available.length)];
    } else {
      state.shuffleHistory = [state.currentQueueIndex];
      nextIdx = Math.floor(Math.random() * state.currentQueue.length);
      if (nextIdx === state.currentQueueIndex && state.currentQueue.length > 1) {
        nextIdx = (nextIdx + 1) % state.currentQueue.length;
      }
    }

    state.shuffleHistory.push(nextIdx);
    if (state.shuffleHistory.length > 8) state.shuffleHistory.shift();
    playQueueIndex(nextIdx);
  }

  async function togglePlayPause() {
    if (!audio.src || !state.currentSong) {
      // Pick first track from active playlist or library
      if (state.selectedPlaylistDetails && state.selectedPlaylistDetails.tracks.length > 0) {
        playSong(state.selectedPlaylistDetails.tracks[0], state.selectedPlaylistDetails.tracks, 0, state.selectedPlaylistId);
      } else if (state.songs.length > 0) {
        playSong(state.songs[0], state.songs, 0, null);
      }
      return;
    }

    if (audio.paused) {
      try {
        await audio.play();
      } catch (err) {
        showToast('Playback failed: ' + err.message, 'error');
      }
    } else {
      audio.pause();
    }
  }

  function nextTrack() {
    if (state.currentQueue.length === 0) return;

    if (state.isShuffle) {
      playNextShuffleTrack();
    } else if (state.currentQueueIndex < state.currentQueue.length - 1) {
      playQueueIndex(state.currentQueueIndex + 1);
    } else if (state.repeatMode === 'all') {
      playQueueIndex(0);
    } else {
      showToast('End of queue reached.', 'warning');
    }
  }

  function previousTrack() {
    if (state.currentQueue.length === 0) return;

    // If more than 3 seconds in, restart track
    if (audio.currentTime > 3.0) {
      audio.currentTime = 0;
      return;
    }

    if (state.currentQueueIndex > 0) {
      playQueueIndex(state.currentQueueIndex - 1);
    } else if (state.repeatMode === 'all') {
      playQueueIndex(state.currentQueue.length - 1);
    } else {
      audio.currentTime = 0;
    }
  }

  function seekToPercent(percent) {
    if (audio.duration && isFinite(audio.duration)) {
      audio.currentTime = (percent / 100) * audio.duration;
    }
  }

  function setVolume(val) {
    val = Math.max(0, Math.min(1, val));
    state.volume = val;
    audio.volume = val;
    audio.muted = (val === 0);
    state.isMuted = (val === 0);

    elements.playerVolumeSlider.value = val;
    elements.playerVolumeValue.textContent = `${Math.round(val * 100)}%`;

    if (val === 0) {
      elements.iconVolHigh.style.display = 'none';
      elements.iconVolMuted.style.display = 'block';
    } else {
      elements.iconVolHigh.style.display = 'block';
      elements.iconVolMuted.style.display = 'none';
    }
  }

  function toggleMute() {
    state.isMuted = !state.isMuted;
    audio.muted = state.isMuted;

    if (state.isMuted) {
      elements.iconVolHigh.style.display = 'none';
      elements.iconVolMuted.style.display = 'block';
      elements.playerVolumeValue.textContent = '0%';
    } else {
      elements.iconVolHigh.style.display = 'block';
      elements.iconVolMuted.style.display = 'none';
      elements.playerVolumeValue.textContent = `${Math.round(audio.volume * 100)}%`;
    }
  }

  function toggleShuffle() {
    state.isShuffle = !state.isShuffle;
    state.shuffleHistory = [];
    if (state.isShuffle) {
      elements.playerShuffleBtn.classList.add('active');
      showToast('Shuffle Mode: ON', 'success');
    } else {
      elements.playerShuffleBtn.classList.remove('active');
      showToast('Shuffle Mode: OFF', 'success');
    }
  }

  function toggleRepeat() {
    if (state.repeatMode === 'off') {
      state.repeatMode = 'all';
      elements.playerRepeatBtn.classList.add('active');
      elements.repeatModeBadge.style.display = 'none';
      showToast('Repeat Mode: ALL TRACKS', 'success');
    } else if (state.repeatMode === 'all') {
      state.repeatMode = 'one';
      elements.playerRepeatBtn.classList.add('active');
      elements.repeatModeBadge.style.display = 'flex';
      showToast('Repeat Mode: CURRENT TRACK', 'success');
    } else {
      state.repeatMode = 'off';
      elements.playerRepeatBtn.classList.remove('active');
      elements.repeatModeBadge.style.display = 'none';
      showToast('Repeat Mode: OFF', 'success');
    }
  }

  function updateActiveRowHighlight() {
    const curId = state.currentSong ? state.currentSong.id : null;

    // Table rows
    document.querySelectorAll('.song-table tbody tr').forEach(tr => {
      const trackId = parseInt(tr.dataset.songId, 10);
      if (trackId === curId) {
        tr.classList.add('playing-row');
        const playBtn = tr.querySelector('.row-play-btn');
        if (playBtn) {
          playBtn.innerHTML = state.isPlaying
            ? `<svg viewBox="0 0 24 24" width="14" height="14" fill="currentColor"><rect x="6" y="4" width="4" height="16"></rect><rect x="14" y="4" width="4" height="16"></rect></svg>`
            : `<svg viewBox="0 0 24 24" width="14" height="14" fill="currentColor"><polygon points="6 4 20 12 6 20 6 4"></polygon></svg>`;
          playBtn.title = state.isPlaying ? 'Pause' : 'Play';
        }
      } else {
        tr.classList.remove('playing-row');
        const playBtn = tr.querySelector('.row-play-btn');
        if (playBtn) {
          playBtn.innerHTML = `<svg viewBox="0 0 24 24" width="14" height="14" fill="currentColor"><polygon points="6 4 20 12 6 20 6 4"></polygon></svg>`;
          playBtn.title = 'Play';
        }
      }
    });

    // Featured tracks grid
    document.querySelectorAll('.track-mini-card').forEach(card => {
      const trackId = parseInt(card.dataset.songId, 10);
      const isCur = (trackId === curId);
      card.classList.toggle('playing-track', isCur);
      const playBtn = card.querySelector('.track-mini-play-btn');
      if (playBtn) {
        if (isCur && state.isPlaying) {
          playBtn.innerHTML = `<svg viewBox="0 0 24 24" width="13" height="13" fill="currentColor"><rect x="6" y="4" width="4" height="16"></rect><rect x="14" y="4" width="4" height="16"></rect></svg>`;
          playBtn.title = 'Pause';
          playBtn.classList.add('playing');
        } else {
          playBtn.innerHTML = `<svg viewBox="0 0 24 24" width="13" height="13" fill="currentColor"><polygon points="6 4 20 12 6 20 6 4"></polygon></svg>`;
          playBtn.title = 'Play';
          playBtn.classList.remove('playing');
        }
      }
    });
  }

  // ── Music Upload Handlers ────────────────────────────────────────────

  function setupUploadHandlers() {
    const dropzone = elements.uploadDropZone;
    const fileInput = elements.uploadAudioInput;

    // Dropzone Click opens file chooser
    dropzone.addEventListener('click', () => fileInput.click());

    // Drag and drop visual cues
    ['dragenter', 'dragover'].forEach(eventName => {
      dropzone.addEventListener(eventName, (e) => {
        e.preventDefault();
        e.stopPropagation();
        dropzone.classList.add('dragover');
      });
    });

    ['dragleave', 'drop'].forEach(eventName => {
      dropzone.addEventListener(eventName, (e) => {
        e.preventDefault();
        e.stopPropagation();
        dropzone.classList.remove('dragover');
      });
    });

    dropzone.addEventListener('drop', (e) => {
      if (e.dataTransfer.files && e.dataTransfer.files.length > 0) {
        handleFileSelection(e.dataTransfer.files[0]);
      }
    });

    fileInput.addEventListener('change', (e) => {
      if (e.target.files && e.target.files.length > 0) {
        handleFileSelection(e.target.files[0]);
      }
    });

    elements.btnClearUploadedFile.addEventListener('click', () => {
      clearUploadSelection();
    });

    // Form submission
    elements.formUploadMusic.addEventListener('submit', async (e) => {
      e.preventDefault();
      if (!state.selectedUploadFile) {
        showUploadError('Please select an audio file to upload.');
        return;
      }

      await executeMusicUpload();
    });
  }

  function handleFileSelection(file) {
    clearUploadError();
    const validExtensions = ['.mp3', '.wav', '.ogg', '.flac'];
    const fileName = file.name;
    const ext = fileName.slice((fileName.lastIndexOf(".") - 1 >>> 0) + 2).toLowerCase();

    if (!validExtensions.includes('.' + ext)) {
      showUploadError(`Invalid file format ".${ext}". Only MP3, WAV, OGG, and FLAC files are supported.`);
      return;
    }

    if (file.size > 50 * 1024 * 1024) {
      showUploadError('File size exceeds the 50 MB limit.');
      return;
    }

    state.selectedUploadFile = file;

    // Update file badge
    elements.uploadDetectedFormat.textContent = ext.toUpperCase();
    elements.uploadSelectedFilename.textContent = file.name;
    elements.uploadSelectedFilesize.textContent = formatBytes(file.size);
    elements.uploadFileMeta.style.display = 'flex';
    elements.btnSubmitUpload.disabled = false;

    // Auto-populate Title & Artist from filename if inputs are empty
    let cleanName = file.name.substring(0, file.name.lastIndexOf('.'));
    cleanName = cleanName.replace(/_/g, ' ');
    if (cleanName.includes(' - ')) {
      const parts = cleanName.split(' - ');
      if (!elements.uploadArtist.value) elements.uploadArtist.value = parts[0].trim();
      if (!elements.uploadTitle.value) elements.uploadTitle.value = parts.slice(1).join(' - ').trim();
    } else {
      if (!elements.uploadTitle.value) elements.uploadTitle.value = cleanName.trim();
    }
    if (!elements.uploadGenre.value) elements.uploadGenre.value = 'Electronic';
    if (!elements.uploadAlbum.value) elements.uploadAlbum.value = 'Single';

    // Client-side exact duration detection via Audio object
    try {
      const blobUrl = URL.createObjectURL(file);
      const probeAudio = new Audio(blobUrl);
      probeAudio.addEventListener('loadedmetadata', () => {
        if (probeAudio.duration && isFinite(probeAudio.duration)) {
          const durSec = Math.round(probeAudio.duration);
          elements.uploadDuration.value = durSec;
          elements.uploadSelectedFilesize.textContent = `${formatBytes(file.size)} • ${formatDuration(durSec)}`;
        }
        URL.revokeObjectURL(blobUrl);
      });
    } catch (e) {}
  }

  function clearUploadSelection() {
    state.selectedUploadFile = null;
    elements.uploadAudioInput.value = '';
    elements.uploadFileMeta.style.display = 'none';
    elements.btnSubmitUpload.disabled = true;
    elements.uploadProgressWrapper.style.display = 'none';
    clearUploadError();
  }

  function showUploadError(msg) {
    elements.uploadErrorAlert.textContent = msg;
    elements.uploadErrorAlert.style.display = 'block';
  }

  function clearUploadError() {
    elements.uploadErrorAlert.textContent = '';
    elements.uploadErrorAlert.style.display = 'none';
  }

  function executeMusicUpload() {
    return new Promise((resolve, reject) => {
      clearUploadError();
      const file = state.selectedUploadFile;
      const formData = new FormData();
      formData.append('file', file, file.name);
      formData.append('title', elements.uploadTitle.value.trim());
      formData.append('artist', elements.uploadArtist.value.trim());
      formData.append('album', elements.uploadAlbum.value.trim());
      formData.append('genre', elements.uploadGenre.value.trim());
      formData.append('artworkUrl', elements.uploadArtwork.value.trim());

      const dur = parseInt(elements.uploadDuration.value, 10);
      if (dur > 0) formData.append('duration', dur);

      elements.uploadProgressWrapper.style.display = 'block';
      elements.btnSubmitUpload.disabled = true;

      const xhr = new XMLHttpRequest();
      xhr.open('POST', `${API_BASE}/api/songs/upload`);

      xhr.upload.onprogress = (e) => {
        if (e.lengthComputable) {
          const percent = Math.round((e.loaded / e.total) * 100);
          elements.uploadProgressBar.style.width = `${percent}%`;
          elements.uploadProgressPercent.textContent = `${percent}%`;
          elements.uploadStatusText.textContent = percent < 100 ? 'Uploading to server...' : 'Processing audio stream...';
        }
      };

      xhr.onload = async () => {
        if (xhr.status >= 200 && xhr.status < 300) {
          try {
            const newSong = JSON.parse(xhr.responseText);
            elements.modalUploadMusic.style.display = 'none';
            clearUploadSelection();
            elements.formUploadMusic.reset();

            await fetchSongs();
            renderGenrePills();
            renderLibraryTable();
            renderHomeView();

            showToast(`Track "${newSong.title}" uploaded successfully!`, 'success');

            // Immediately offer to play the uploaded track
            playSong(newSong, state.songs, state.songs.findIndex(s => s.id === newSong.id));
            resolve(newSong);
          } catch (err) {
            showUploadError('Failed to parse server response.');
            elements.btnSubmitUpload.disabled = false;
            reject(err);
          }
        } else {
          try {
            const errRes = JSON.parse(xhr.responseText);
            showUploadError(errRes.error || `Upload failed (Status ${xhr.status})`);
          } catch {
            showUploadError(`Server returned error ${xhr.status}: ${xhr.statusText}`);
          }
          elements.btnSubmitUpload.disabled = false;
          elements.uploadProgressWrapper.style.display = 'none';
          reject(new Error(xhr.statusText));
        }
      };

      xhr.onerror = () => {
        showUploadError('Network error occurred during audio upload.');
        elements.btnSubmitUpload.disabled = false;
        elements.uploadProgressWrapper.style.display = 'none';
        reject(new Error('Network error'));
      };

      xhr.send(formData);
    });
  }

  // ── Edit Song Metadata Handlers ──────────────────────────────────────

  function openEditSongModal(song) {
    elements.editSongId.value = song.id;
    elements.editSongSubtitle.textContent = `Editing track #${song.id} (${song.hasAudio ? 'Real Audio Attached' : 'Catalog'})`;
    elements.editSongTitle.value = song.title;
    elements.editSongArtist.value = song.artist;
    elements.editSongAlbum.value = song.album || 'Single';
    elements.editSongGenre.value = song.genre;
    elements.editSongDuration.value = song.duration;
    elements.editSongArtwork.value = song.artworkUrl || '';
    elements.modalEditSong.style.display = 'flex';
  }

  async function handleEditSongSubmit(e) {
    e.preventDefault();
    const id = parseInt(elements.editSongId.value, 10);
    const payload = {
      title: elements.editSongTitle.value.trim(),
      artist: elements.editSongArtist.value.trim(),
      album: elements.editSongAlbum.value.trim(),
      genre: elements.editSongGenre.value.trim(),
      duration: parseInt(elements.editSongDuration.value, 10),
      artworkUrl: elements.editSongArtwork.value.trim()
    };

    try {
      const updated = await apiFetch(`/api/songs/${id}`, {
        method: 'PUT',
        body: JSON.stringify(payload)
      });

      elements.modalEditSong.style.display = 'none';
      await fetchSongs();
      await fetchPlaylists();
      renderGenrePills();
      renderLibraryTable();
      renderHomeView();
      if (state.selectedPlaylistId !== null) {
        await selectPlaylist(state.selectedPlaylistId, false);
      }

      // If this song is currently playing, update bottom player display
      if (state.currentSong && state.currentSong.id === id) {
        state.currentSong = updated;
        elements.playerTrackTitle.textContent = updated.title;
        elements.playerTrackArtist.textContent = updated.artist;
        if (updated.album && updated.album !== 'Single') {
          elements.playerTrackAlbum.textContent = updated.album;
          elements.playerTrackAlbum.style.display = 'inline';
          elements.trackAlbumSep.style.display = 'inline';
        }
      }

      showToast(`Track "${updated.title}" updated successfully!`, 'success');
    } catch (err) {
      showToast('Update failed: ' + err.message, 'error');
    }
  }

  // ── Global Keyboard Shortcuts ────────────────────────────────────────

  function setupKeyboardShortcuts() {
    window.addEventListener('keydown', (e) => {
      // Ignore shortcut keys when typing in inputs or textareas
      const tag = (document.activeElement || {}).tagName;
      if (tag === 'INPUT' || tag === 'TEXTAREA' || tag === 'SELECT') {
        return;
      }

      switch (e.code) {
        case 'Space':
          e.preventDefault();
          togglePlayPause();
          break;
        case 'ArrowLeft':
          e.preventDefault();
          if (audio.duration) {
            audio.currentTime = Math.max(0, audio.currentTime - 5);
          }
          break;
        case 'ArrowRight':
          e.preventDefault();
          if (audio.duration) {
            audio.currentTime = Math.min(audio.duration, audio.currentTime + 5);
          }
          break;
        case 'ArrowUp':
          e.preventDefault();
          setVolume(Math.min(1.0, state.volume + 0.05));
          break;
        case 'ArrowDown':
          e.preventDefault();
          setVolume(Math.max(0.0, state.volume - 0.05));
          break;
        case 'KeyM':
          toggleMute();
          break;
        case 'KeyN':
          nextTrack();
          break;
        case 'KeyP':
          previousTrack();
          break;
        case 'KeyS':
          toggleShuffle();
          break;
        case 'KeyR':
          toggleRepeat();
          break;
      }
    });
  }

  // ── Data Fetching ────────────────────────────────────────────────────

  async function fetchSongs() {
    const sortBy = elements.librarySortSelect ? elements.librarySortSelect.value : 'id';
    state.songs = await apiFetch(`/api/songs?sortBy=${sortBy}`);
    updateStatsBar();
  }

  async function fetchPlaylists() {
    state.playlists = await apiFetch('/api/playlists');
    renderSidebarPlaylists();
    updateStatsBar();
  }

  function updateStatsBar() {
    elements.statSongsCount.textContent = state.songs.length;
    elements.statPlaylistsCount.textContent = state.playlists.length;

    const totalSeconds = state.songs.reduce((acc, s) => acc + s.duration, 0);
    const hrs = Math.floor(totalSeconds / 3600);
    const mins = Math.floor((totalSeconds % 3600) / 60);
    elements.statTotalDuration.textContent = hrs > 0 ? `${hrs}h ${mins}m` : `${mins} mins`;
  }

  // ── Views Navigation ─────────────────────────────────────────────────

  function switchView(viewName) {
    state.activeView = viewName;
    elements.navItems.forEach(item => {
      item.classList.toggle('active', item.dataset.view === viewName);
    });
    elements.viewPanels.forEach(panel => {
      panel.classList.remove('active');
    });

    const targetPanel = document.getElementById(`view${capitalize(viewName)}`);
    if (targetPanel) targetPanel.classList.add('active');

    if (viewName === 'home') renderHomeView();
    else if (viewName === 'library') {
      renderGenrePills();
      renderLibraryTable();
    } else if (viewName === 'playlists') {
      renderPlaylistsView();
    } else if (viewName === 'search') {
      renderSearchView();
    }
  }

  function capitalize(str) {
    return str.charAt(0).toUpperCase() + str.slice(1);
  }

  // ── Home View Rendering ──────────────────────────────────────────────

  function renderHomeView() {
    // 1. Playlists Shelf
    elements.homePlaylistsShelf.innerHTML = '';
    if (!state.playlists || state.playlists.length === 0) {
      elements.homePlaylistsShelf.innerHTML = `
        <div class="shelf-empty-hint">
          <p>No playlists created yet.</p>
          <button class="btn btn-secondary btn-sm" id="btnHomeCreatePl">Create Playlist</button>
        </div>
      `;
      const btn = document.getElementById('btnHomeCreatePl');
      if (btn) {
        btn.addEventListener('click', () => {
          elements.inputPlaylistName.value = '';
          elements.modalCreatePlaylist.style.display = 'flex';
        });
      }
    } else {
      state.playlists.slice(0, 6).forEach(pl => {
        const card = document.createElement('div');
        const isSelected = state.selectedPlaylistId === pl.id;
        card.className = `playlist-card ${isSelected ? 'active' : ''}`;
        card.setAttribute('data-playlist-id', pl.id);
        const trackWord = pl.trackCount === 1 ? 'track' : 'tracks';
        card.innerHTML = `
          <div class="playlist-card-artwork">
            🎵
            <button class="playlist-card-play-btn" title="Play ${escapeHtml(pl.name)}" aria-label="Play ${escapeHtml(pl.name)}" data-play-playlist="${pl.id}">
              <svg viewBox="0 0 24 24" width="18" height="18" fill="currentColor">
                <polygon points="5 3 19 12 5 21 5 3"></polygon>
              </svg>
            </button>
          </div>
          <div class="playlist-card-name" title="${escapeHtml(pl.name)}">${escapeHtml(pl.name)}</div>
          <div class="playlist-card-tracks">${pl.trackCount} ${trackWord} &bull; ${pl.formattedDuration}</div>
        `;
        card.addEventListener('click', () => {
          selectPlaylist(pl.id, true);
        });
        const playBtn = card.querySelector('[data-play-playlist]');
        if (playBtn) {
          playBtn.addEventListener('click', (e) => {
            e.stopPropagation();
            playPlaylistById(pl.id);
          });
        }
        elements.homePlaylistsShelf.appendChild(card);
      });
    }

    // 2. Featured Tracks Grid
    elements.homeFeaturedTracksGrid.innerHTML = '';
    if (!state.songs || state.songs.length === 0) {
      elements.homeFeaturedTracksGrid.innerHTML = `
        <div class="shelf-empty-hint">
          <p>Your library has no tracks yet.</p>
          <button class="btn btn-primary btn-sm" id="btnHomeUpload">Upload Audio Files</button>
        </div>
      `;
      const btn = document.getElementById('btnHomeUpload');
      if (btn) {
        btn.addEventListener('click', () => {
          clearUploadSelection();
          elements.modalUploadMusic.style.display = 'flex';
        });
      }
    } else {
      const curId = state.currentSong ? state.currentSong.id : null;
      state.songs.slice(0, 6).forEach(song => {
        const item = document.createElement('div');
        const isCur = (song.id === curId);
        item.className = `track-mini-card ${isCur ? 'playing-track' : ''}`;
        item.setAttribute('data-song-id', song.id);

        const artworkHtml = song.artworkUrl
          ? `<img src="${escapeHtml(song.artworkUrl)}" alt="${escapeHtml(song.title)}" class="track-mini-img">`
          : `<svg viewBox="0 0 24 24" width="18" height="18" fill="currentColor"><path d="M12 3v10.55c-.59-.34-1.27-.55-2-.55-2.21 0-4 1.79-4 4s1.79 4 4 4 4-1.79 4-4V7h4V3h-6z"/></svg>`;

        const isPlayingThis = isCur && state.isPlaying;

        item.innerHTML = `
          <div class="track-mini-meta">
            <div class="track-mini-icon">
              ${artworkHtml}
            </div>
            <div class="track-mini-texts">
              <div class="track-mini-title" title="${escapeHtml(song.title)}">${escapeHtml(song.title)}</div>
              <div class="track-mini-artist" title="${escapeHtml(song.artist)}">${escapeHtml(song.artist)}</div>
            </div>
          </div>
          <div class="track-mini-right">
            <span class="track-mini-duration">${song.formattedDuration}</span>
            <button class="track-mini-play-btn ${isPlayingThis ? 'playing' : ''}" title="${isPlayingThis ? 'Pause' : 'Play'}" aria-label="Play ${escapeHtml(song.title)}">
              ${isPlayingThis
                ? `<svg viewBox="0 0 24 24" width="13" height="13" fill="currentColor"><rect x="6" y="4" width="4" height="16"></rect><rect x="14" y="4" width="4" height="16"></rect></svg>`
                : `<svg viewBox="0 0 24 24" width="13" height="13" fill="currentColor"><polygon points="6 4 20 12 6 20 6 4"></polygon></svg>`}
            </button>
          </div>
        `;

        item.addEventListener('click', () => {
          if (state.currentSong && state.currentSong.id === song.id) {
            togglePlayPause();
          } else {
            playSong(song, state.songs, state.songs.findIndex(s => s.id === song.id), null);
          }
        });

        const playBtn = item.querySelector('.track-mini-play-btn');
        if (playBtn) {
          playBtn.addEventListener('click', (e) => {
            e.stopPropagation();
            if (state.currentSong && state.currentSong.id === song.id) {
              togglePlayPause();
            } else {
              playSong(song, state.songs, state.songs.findIndex(s => s.id === song.id), null);
            }
          });
        }

        elements.homeFeaturedTracksGrid.appendChild(item);
      });
    }
  }

  // ── Library View Rendering ───────────────────────────────────────────

  function renderGenrePills() {
    const genres = Array.from(new Set(state.songs.map(s => s.genre))).sort();
    elements.genrePillsContainer.innerHTML = '';

    const allBtn = document.createElement('button');
    allBtn.className = `pill ${state.genreFilter === 'all' ? 'active' : ''}`;
    allBtn.textContent = 'All Genres';
    allBtn.addEventListener('click', () => {
      state.genreFilter = 'all';
      renderGenrePills();
      renderLibraryTable();
    });
    elements.genrePillsContainer.appendChild(allBtn);

    genres.forEach(g => {
      const btn = document.createElement('button');
      btn.className = `pill ${state.genreFilter === g ? 'active' : ''}`;
      btn.textContent = g;
      btn.addEventListener('click', () => {
        state.genreFilter = g;
        renderGenrePills();
        renderLibraryTable();
      });
      elements.genrePillsContainer.appendChild(btn);
    });
  }

  function renderLibraryTable() {
    elements.masterSongTableBody.innerHTML = '';
    const filtered = state.genreFilter === 'all'
      ? state.songs
      : state.songs.filter(s => s.genre === state.genreFilter);

    if (!filtered || filtered.length === 0) {
      if (elements.masterSongTable) elements.masterSongTable.style.display = 'none';
      elements.libraryEmptyState.style.display = 'block';
      const emptyH3 = elements.libraryEmptyState.querySelector('h3');
      const emptyP = elements.libraryEmptyState.querySelector('p');
      if (state.songs.length > 0) {
        if (emptyH3) emptyH3.textContent = `No tracks in "${state.genreFilter}"`;
        if (emptyP) emptyP.textContent = 'Try choosing "All Genres" or adding tracks under this genre.';
      } else {
        if (emptyH3) emptyH3.textContent = 'Your Music Library is Empty';
        if (emptyP) emptyP.textContent = 'Upload your local audio files (MP3, WAV, OGG, or FLAC) to build your personal streaming library.';
      }
      return;
    }
    if (elements.masterSongTable) elements.masterSongTable.style.display = 'table';
    elements.libraryEmptyState.style.display = 'none';

    filtered.forEach((song, idx) => {
      const isCur = state.currentSong && state.currentSong.id === song.id;
      const tr = document.createElement('tr');
      tr.dataset.songId = song.id;
      if (isCur) tr.classList.add('playing-row');

      tr.innerHTML = `
        <td class="track-num">
          <button class="row-play-btn" title="Play track" data-play-id="${song.id}">
            ${isCur && state.isPlaying 
              ? `<svg viewBox="0 0 24 24" width="14" height="14" fill="currentColor"><rect x="6" y="4" width="4" height="16"></rect><rect x="14" y="4" width="4" height="16"></rect></svg>`
              : `<svg viewBox="0 0 24 24" width="14" height="14" fill="currentColor"><polygon points="6 4 20 12 6 20 6 4"></polygon></svg>`}
          </button>
          <span class="track-index-num">${idx + 1}</span>
        </td>
        <td>
          <div class="title-cell-flex">
            <div class="song-thumb-art">
              ${song.artworkUrl ? `<img src="${escapeHtml(song.artworkUrl)}">` : '🎵'}
            </div>
            <div class="title-cell">
              <span class="song-title-text">${escapeHtml(song.title)}</span>
              <span class="song-artist-text">${escapeHtml(song.artist)}</span>
            </div>
          </div>
        </td>
        <td class="album-cell"><span class="album-tag">${escapeHtml(song.album || 'Single')}</span></td>
        <td><span class="genre-tag">${escapeHtml(song.genre)}</span></td>
        <td class="time-cell" style="text-align: right;">${song.formattedDuration}</td>
        <td class="actions-cell">
          <button class="btn btn-secondary btn-icon" title="Add to Playlist" data-add-song="${song.id}">
            +
          </button>
          <button class="btn btn-secondary btn-icon" title="Edit Metadata" data-edit-song="${song.id}">
            ✎
          </button>
          <button class="btn btn-danger btn-icon" title="Delete Song" data-del-song="${song.id}">
            🗑
          </button>
        </td>
      `;

      // Play row button & row double-click
      tr.querySelector('[data-play-id]').addEventListener('click', (e) => {
        e.stopPropagation();
        if (state.currentSong && state.currentSong.id === song.id) {
          togglePlayPause();
        } else {
          playSong(song, filtered, idx, null);
        }
      });

      tr.addEventListener('dblclick', () => {
        playSong(song, filtered, idx, null);
      });

      // Actions
      tr.querySelector('[data-add-song]').addEventListener('click', (e) => {
        e.stopPropagation();
        openAddToPlaylistModal(song);
      });

      tr.querySelector('[data-edit-song]').addEventListener('click', (e) => {
        e.stopPropagation();
        openEditSongModal(song);
      });

      tr.querySelector('[data-del-song]').addEventListener('click', (e) => {
        e.stopPropagation();
        deleteSongFromLibrary(song.id, song.title);
      });

      elements.masterSongTableBody.appendChild(tr);
    });
  }

  // ── Playlists View Rendering ─────────────────────────────────────────

  function renderSidebarPlaylists() {
    elements.sidebarPlaylistsList.innerHTML = '';
    if (!state.playlists || state.playlists.length === 0) {
      elements.sidebarPlaylistsList.innerHTML = `
        <div class="sidebar-empty-playlists">
          <span>No playlists created yet</span>
        </div>
      `;
      return;
    }

    state.playlists.forEach(pl => {
      const isSelected = state.selectedPlaylistId === pl.id;
      const item = document.createElement('div');
      item.className = `sidebar-playlist-item ${isSelected ? 'active' : ''}`;
      item.setAttribute('role', 'button');
      item.setAttribute('tabindex', '0');
      item.setAttribute('data-playlist-id', pl.id);

      const trackWord = pl.trackCount === 1 ? 'track' : 'tracks';
      item.setAttribute('title', `${pl.name} • ${pl.trackCount} ${trackWord} (ID: ${pl.id})`);
      item.setAttribute('aria-label', `${pl.name}, ${pl.trackCount} ${trackWord}`);

      item.innerHTML = `
        <div class="sidebar-pl-left">
          <div class="sidebar-pl-icon" aria-hidden="true">
            <svg viewBox="0 0 24 24" width="13" height="13" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
              <polygon points="12 2 2 7 12 12 22 7 12 2"></polygon>
              <polyline points="2 17 12 22 22 17"></polyline>
              <polyline points="2 12 12 17 22 12"></polyline>
            </svg>
          </div>
          <span class="sidebar-pl-name">${escapeHtml(pl.name)}</span>
        </div>
        <span class="sidebar-pl-badge" title="${pl.trackCount} ${trackWord}">${pl.trackCount}</span>
      `;

      item.addEventListener('click', () => {
        selectPlaylist(pl.id, true);
      });

      item.addEventListener('keydown', (e) => {
        if (e.key === 'Enter' || e.key === ' ') {
          e.preventDefault();
          selectPlaylist(pl.id, true);
        }
      });

      elements.sidebarPlaylistsList.appendChild(item);
    });
  }

  function renderPlaylistsView() {
    elements.allPlaylistsGrid.innerHTML = '';

    if (!state.playlists || state.playlists.length === 0) {
      elements.allPlaylistsGrid.innerHTML = `
        <div class="empty-state" style="grid-column: 1 / -1;">
          <div class="empty-icon">📑</div>
          <h3>No Playlists Yet</h3>
          <p>Synthesize your first custom playlist or load an existing CSV playlist file.</p>
          <button class="btn btn-primary" id="btnEmptyCreatePlaylist">
            <svg viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
              <line x1="12" y1="5" x2="12" y2="19"></line>
              <line x1="5" y1="12" x2="19" y2="12"></line>
            </svg>
            <span>Create New Playlist</span>
          </button>
        </div>
      `;
      const btn = document.getElementById('btnEmptyCreatePlaylist');
      if (btn) {
        btn.addEventListener('click', () => {
          elements.inputPlaylistName.value = '';
          elements.modalCreatePlaylist.style.display = 'flex';
        });
      }
      elements.playlistDetailContainer.style.display = 'none';
      return;
    }

    state.playlists.forEach(pl => {
      const card = document.createElement('div');
      card.className = `playlist-card ${state.selectedPlaylistId === pl.id ? 'active' : ''}`;
      card.innerHTML = `
        <div class="playlist-card-artwork">🎵</div>
        <button class="playlist-card-play-btn" title="Play playlist" data-play-playlist="${pl.id}">
          <svg viewBox="0 0 24 24" width="18" height="18" fill="currentColor">
            <polygon points="5 3 19 12 5 21 5 3"></polygon>
          </svg>
        </button>
        <div class="playlist-card-name">${escapeHtml(pl.name)}</div>
        <div class="playlist-card-tracks">${pl.trackCount} tracks &bull; ${pl.formattedDuration}</div>
      `;
      card.addEventListener('click', () => {
        selectPlaylist(pl.id, false);
      });
      const playBtn = card.querySelector('[data-play-playlist]');
      if (playBtn) {
        playBtn.addEventListener('click', (e) => {
          e.stopPropagation();
          playPlaylistById(pl.id);
        });
      }
      elements.allPlaylistsGrid.appendChild(card);
    });

    if (state.selectedPlaylistId !== null) {
      elements.playlistDetailContainer.style.display = 'block';
      if (!state.selectedPlaylistDetails || state.selectedPlaylistDetails.id !== state.selectedPlaylistId) {
        selectPlaylist(state.selectedPlaylistId, false);
      } else {
        renderActivePlaylistDetails();
      }
    } else {
      elements.playlistDetailContainer.style.display = 'none';
    }
  }

  async function playPlaylistById(id) {
    try {
      const details = await apiFetch(`/api/playlists/${id}`);
      if (details && details.tracks && details.tracks.length > 0) {
        playSong(details.tracks[0], details.tracks, 0, id);
      } else {
        showToast('Playlist has no tracks to play.', 'warning');
      }
    } catch (err) {
      showToast('Could not play playlist: ' + err.message, 'error');
    }
  }

  async function selectPlaylist(id, autoSwitchView = true) {
    state.selectedPlaylistId = id;
    try {
      state.selectedPlaylistDetails = await apiFetch(`/api/playlists/${id}`);
      renderSidebarPlaylists();
      if (autoSwitchView && state.activeView !== 'playlists') {
        switchView('playlists');
      } else {
        renderPlaylistsView();
      }
    } catch (err) {
      showToast('Could not load playlist: ' + err.message, 'error');
    }
  }

  function renderActivePlaylistDetails() {
    const d = state.selectedPlaylistDetails;
    if (!d) return;

    elements.detailTitle.textContent = d.name;
    elements.detailStats.textContent = `${d.trackCount} tracks • Total playing time: ${d.formattedDuration}`;

    elements.playlistTracksTableBody.innerHTML = '';
    if (!d.tracks || d.tracks.length === 0) {
      if (elements.playlistTracksTable) elements.playlistTracksTable.style.display = 'none';
      elements.playlistTracksEmptyState.style.display = 'block';
      return;
    }
    if (elements.playlistTracksTable) elements.playlistTracksTable.style.display = 'table';
    elements.playlistTracksEmptyState.style.display = 'none';

    d.tracks.forEach((track, index) => {
      const isCur = state.currentSong && state.currentSong.id === track.id;
      const tr = document.createElement('tr');
      tr.dataset.songId = track.id;
      if (isCur) tr.classList.add('playing-row');

      tr.innerHTML = `
        <td class="track-num">
          <button class="row-play-btn" data-play-idx="${index}">
            ${isCur && state.isPlaying 
              ? `<svg viewBox="0 0 24 24" width="14" height="14" fill="currentColor"><rect x="6" y="4" width="4" height="16"></rect><rect x="14" y="4" width="4" height="16"></rect></svg>`
              : `<svg viewBox="0 0 24 24" width="14" height="14" fill="currentColor"><polygon points="6 4 20 12 6 20 6 4"></polygon></svg>`}
          </button>
          <span>${index + 1}</span>
        </td>
        <td>
          <div class="title-cell-flex">
            <div class="song-thumb-art">
              ${track.artworkUrl ? `<img src="${escapeHtml(track.artworkUrl)}">` : '🎵'}
            </div>
            <div class="title-cell">
              <span class="song-title-text">${escapeHtml(track.title)}</span>
              <span class="song-artist-text">${escapeHtml(track.artist)}</span>
            </div>
          </div>
        </td>
        <td><span class="genre-tag">${escapeHtml(track.genre)}</span></td>
        <td class="time-cell" style="text-align: right;">${track.formattedDuration}</td>
        <td class="actions-cell">
          <button class="btn btn-danger btn-icon" title="Remove from playlist" data-remove-track="${track.id}">
            ✕
          </button>
        </td>
      `;

      tr.querySelector('[data-play-idx]').addEventListener('click', (e) => {
        e.stopPropagation();
        if (state.currentSong && state.currentSong.id === track.id) {
          togglePlayPause();
        } else {
          playSong(track, d.tracks, index, state.selectedPlaylistId);
        }
      });

      tr.addEventListener('dblclick', () => {
        playSong(track, d.tracks, index, state.selectedPlaylistId);
      });

      tr.querySelector('[data-remove-track]').addEventListener('click', (e) => {
        e.stopPropagation();
        removeTrackFromPlaylist(state.selectedPlaylistId, track.id, track.title);
      });

      elements.playlistTracksTableBody.appendChild(tr);
    });
  }

  // ── Playlist Actions ─────────────────────────────────────────────────

  async function shuffleActivePlaylist() {
    if (state.selectedPlaylistId === null) return;
    try {
      const updated = await apiFetch(`/api/playlists/${state.selectedPlaylistId}/shuffle`, { method: 'POST' });
      state.selectedPlaylistDetails = updated;
      renderActivePlaylistDetails();
      showToast(`Shuffled "${updated.name}"!`, 'success');
    } catch (err) {
      showToast('Shuffle error: ' + err.message, 'error');
    }
  }

  async function sortActivePlaylist() {
    if (state.selectedPlaylistId === null) return;
    try {
      const updated = await apiFetch(`/api/playlists/${state.selectedPlaylistId}/sort`, {
        method: 'POST',
        body: JSON.stringify({ sortBy: 'title' })
      });
      state.selectedPlaylistDetails = updated;
      renderActivePlaylistDetails();
      showToast('Sorted by title (A-Z).', 'success');
    } catch (err) {
      showToast('Sort error: ' + err.message, 'error');
    }
  }

  async function saveActivePlaylist() {
    if (state.selectedPlaylistId === null) return;
    try {
      const res = await apiFetch(`/api/playlists/${state.selectedPlaylistId}/save`, { method: 'POST', body: '{}' });
      showToast(`Saved to ${res.filepath}`, 'success');
    } catch (err) {
      showToast('Save failed: ' + err.message, 'error');
    }
  }

  async function renameActivePlaylist() {
    if (state.selectedPlaylistId === null) return;
    const currentName = state.selectedPlaylistDetails.name;
    const newName = prompt('Enter new playlist name:', currentName);
    if (!newName || newName.trim() === '' || newName.trim() === currentName) return;

    try {
      await apiFetch(`/api/playlists/${state.selectedPlaylistId}`, {
        method: 'PUT',
        body: JSON.stringify({ name: newName.trim() })
      });
      await fetchPlaylists();
      await selectPlaylist(state.selectedPlaylistId, false);
      showToast('Playlist renamed successfully!', 'success');
    } catch (err) {
      showToast('Rename failed: ' + err.message, 'error');
    }
  }

  async function deleteActivePlaylist() {
    if (state.selectedPlaylistId === null) return;
    if (!confirm(`Are you sure you want to delete playlist "${state.selectedPlaylistDetails.name}"?`)) return;

    try {
      await apiFetch(`/api/playlists/${state.selectedPlaylistId}`, { method: 'DELETE' });
      state.selectedPlaylistId = null;
      state.selectedPlaylistDetails = null;
      await fetchPlaylists();
      if (state.playlists.length > 0) {
        selectPlaylist(state.playlists[0].id, false);
      } else {
        renderPlaylistsView();
      }
      showToast('Playlist deleted.', 'success');
    } catch (err) {
      showToast('Delete failed: ' + err.message, 'error');
    }
  }

  async function removeTrackFromPlaylist(plId, trackId, trackTitle) {
    try {
      const updated = await apiFetch(`/api/playlists/${plId}/songs/${trackId}`, { method: 'DELETE' });
      state.selectedPlaylistDetails = updated;
      renderActivePlaylistDetails();
      await fetchPlaylists();
      showToast(`Removed "${trackTitle}" from playlist.`, 'success');
    } catch (err) {
      showToast('Failed to remove track: ' + err.message, 'error');
    }
  }

  function openAddToPlaylistModal(song) {
    state.targetSongForPlaylist = song;
    elements.pickerSongPreview.textContent = `Track: "${song.title}" by ${song.artist} (${song.formattedDuration})`;

    elements.selectTargetPlaylist.innerHTML = '';
    state.playlists.forEach(pl => {
      const opt = document.createElement('option');
      opt.value = pl.id;
      opt.textContent = `${pl.name} (${pl.trackCount} tracks)`;
      elements.selectTargetPlaylist.appendChild(opt);
    });

    elements.modalAddToPlaylist.style.display = 'flex';
  }

  async function confirmAddTrackToPlaylist() {
    if (!state.targetSongForPlaylist) return;
    const plId = parseInt(elements.selectTargetPlaylist.value, 10);
    const song = state.targetSongForPlaylist;

    try {
      const updated = await apiFetch(`/api/playlists/${plId}/songs`, {
        method: 'POST',
        body: JSON.stringify({ songId: song.id })
      });

      elements.modalAddToPlaylist.style.display = 'none';
      await fetchPlaylists();
      if (state.selectedPlaylistId === plId) {
        state.selectedPlaylistDetails = updated;
        renderActivePlaylistDetails();
      }

      showToast(`Added "${song.title}" to playlist!`, 'success');
    } catch (err) {
      showToast(err.message, 'warning');
    }
  }

  async function deleteSongFromLibrary(id, title) {
    if (!confirm(`Are you sure you want to delete "${title}" (Track #${id}) and its audio file from master library?`)) return;

    try {
      await apiFetch(`/api/songs/${id}`, { method: 'DELETE' });
      if (state.currentSong && state.currentSong.id === id) {
        audio.pause();
        audio.src = '';
        state.currentSong = null;
        elements.playerTrackTitle.textContent = 'No Track Selected';
        elements.playerTrackArtist.textContent = 'Choose a song to play';
      }

      await fetchSongs();
      await fetchPlaylists();
      renderGenrePills();
      renderLibraryTable();
      renderHomeView();
      if (state.selectedPlaylistId !== null) {
        await selectPlaylist(state.selectedPlaylistId, false);
      }
      showToast(`Deleted track #${id} from library.`, 'success');
    } catch (err) {
      showToast('Delete failed: ' + err.message, 'error');
    }
  }

  // ── Search View Handling ─────────────────────────────────────────────
  let searchDebounceTimer = null;

  async function renderSearchView() {
    if (state.searchQuery) {
      elements.searchResultsHeading.textContent = `Search results for "${state.searchQuery}"`;
      elements.searchResultsSubtitle.textContent = 'Showing matches from live backend query.';
      try {
        const results = await apiFetch(`/api/songs?q=${encodeURIComponent(state.searchQuery)}`);
        renderSearchTable(results);
      } catch (err) {
        showToast('Search query failed: ' + err.message, 'error');
      }
    } else {
      elements.searchResultsHeading.textContent = 'Master Library Search';
      elements.searchResultsSubtitle.textContent = state.songs && state.songs.length > 0
        ? 'Type in the top bar search box to filter, or browse all library songs below.'
        : 'Type in the top bar search box to find songs across title, artist, or genre.';
      renderSearchTable(state.songs);
      if (elements.globalSearchInput) {
        elements.globalSearchInput.focus();
      }
    }
  }

  function handleSearchInput(e) {
    const val = e.target.value.trim();
    state.searchQuery = val;
    elements.searchClearBtn.style.display = val ? 'block' : 'none';

    clearTimeout(searchDebounceTimer);
    searchDebounceTimer = setTimeout(async () => {
      if (val) {
        if (state.activeView !== 'search') {
          switchView('search');
        } else {
          renderSearchView();
        }
      } else if (state.activeView === 'search') {
        renderSearchView();
      }
    }, 200);
  }

  function renderSearchTable(results) {
    elements.searchTableBody.innerHTML = '';
    if (!results || results.length === 0) {
      if (elements.searchTable) elements.searchTable.style.display = 'none';
      elements.searchEmptyState.style.display = 'block';
      const emptyP = elements.searchEmptyState.querySelector('p');
      if (emptyP) {
        emptyP.textContent = state.searchQuery
          ? `No songs found matching "${state.searchQuery}".`
          : 'No songs available in your library.';
      }
      return;
    }
    if (elements.searchTable) elements.searchTable.style.display = 'table';
    elements.searchEmptyState.style.display = 'none';

    results.forEach((song, idx) => {
      const isCur = state.currentSong && state.currentSong.id === song.id;
      const tr = document.createElement('tr');
      tr.dataset.songId = song.id;
      if (isCur) tr.classList.add('playing-row');

      tr.innerHTML = `
        <td class="track-num">
          <button class="row-play-btn" title="Play track" data-play-id="${song.id}">
            <svg viewBox="0 0 24 24" width="14" height="14" fill="currentColor"><polygon points="6 4 20 12 6 20 6 4"></polygon></svg>
          </button>
          <span>${idx + 1}</span>
        </td>
        <td>
          <div class="title-cell-flex">
            <div class="song-thumb-art">
              ${song.artworkUrl ? `<img src="${escapeHtml(song.artworkUrl)}">` : '🎵'}
            </div>
            <div class="title-cell">
              <span class="song-title-text">${escapeHtml(song.title)}</span>
              <span class="song-artist-text">${escapeHtml(song.artist)}</span>
            </div>
          </div>
        </td>
        <td><span class="genre-tag">${escapeHtml(song.genre)}</span></td>
        <td class="time-cell" style="text-align: right;">${song.formattedDuration}</td>
        <td class="actions-cell">
          <button class="btn btn-secondary btn-icon" title="Add to Playlist" data-add-song="${song.id}">
            +
          </button>
        </td>
      `;

      tr.querySelector('[data-play-id]').addEventListener('click', (e) => {
        e.stopPropagation();
        playSong(song, results, idx, null);
      });

      tr.querySelector('[data-add-song]').addEventListener('click', (e) => {
        e.stopPropagation();
        openAddToPlaylistModal(song);
      });

      elements.searchTableBody.appendChild(tr);
    });
  }

  // ── Event Listeners Registration ─────────────────────────────────────

  function setupEventListeners() {
    // Navigation
    elements.navItems.forEach(item => {
      item.addEventListener('click', () => switchView(item.dataset.view));
    });

    elements.heroExploreLibraryBtn.addEventListener('click', () => switchView('library'));
    elements.linkViewAllTracks.addEventListener('click', () => switchView('library'));
    elements.linkViewAllPlaylists.addEventListener('click', () => switchView('playlists'));
    elements.btnEmptyAddFromLibrary.addEventListener('click', () => switchView('library'));

    elements.heroPlayActiveBtn.addEventListener('click', () => {
      if (state.selectedPlaylistDetails && state.selectedPlaylistDetails.tracks.length > 0) {
        playSong(state.selectedPlaylistDetails.tracks[0], state.selectedPlaylistDetails.tracks, 0, state.selectedPlaylistId);
      } else if (state.songs.length > 0) {
        playSong(state.songs[0], state.songs, 0, null);
      } else {
        showToast('No tracks available in library to play.', 'warning');
      }
    });

    // Topbar Search
    elements.globalSearchInput.addEventListener('input', handleSearchInput);
    elements.searchClearBtn.addEventListener('click', () => {
      elements.globalSearchInput.value = '';
      elements.searchClearBtn.style.display = 'none';
      switchView('home');
    });

    elements.btnRefreshAll.addEventListener('click', async () => {
      await fetchSongs();
      await fetchPlaylists();
      renderHomeView();
      showToast('Synchronized with C++17 server.', 'success');
    });

    // Library Sort
    elements.librarySortSelect.addEventListener('change', async () => {
      await fetchSongs();
      renderLibraryTable();
    });

    // Upload Openers
    elements.btnOpenUploadModal.addEventListener('click', () => {
      clearUploadSelection();
      elements.modalUploadMusic.style.display = 'flex';
    });
    elements.btnLibraryUpload.addEventListener('click', () => {
      clearUploadSelection();
      elements.modalUploadMusic.style.display = 'flex';
    });
    elements.btnEmptyUploadMusic.addEventListener('click', () => {
      clearUploadSelection();
      elements.modalUploadMusic.style.display = 'flex';
    });

    // Shortcuts Opener
    elements.btnShortcutsInfo.addEventListener('click', () => {
      elements.modalShortcuts.style.display = 'flex';
    });

    // Edit Song form submit
    elements.formEditSong.addEventListener('submit', handleEditSongSubmit);

    // Playlist Detail Actions
    elements.btnDetailPlay.addEventListener('click', () => {
      if (state.selectedPlaylistDetails && state.selectedPlaylistDetails.tracks.length > 0) {
        playSong(state.selectedPlaylistDetails.tracks[0], state.selectedPlaylistDetails.tracks, 0, state.selectedPlaylistId);
      }
    });
    elements.btnDetailShuffle.addEventListener('click', shuffleActivePlaylist);
    elements.btnDetailSort.addEventListener('click', sortActivePlaylist);
    elements.btnDetailSave.addEventListener('click', saveActivePlaylist);
    elements.btnDetailRename.addEventListener('click', renameActivePlaylist);
    elements.btnDetailDelete.addEventListener('click', deleteActivePlaylist);

    // Bottom Player Controls
    elements.playerPlayPauseBtn.addEventListener('click', togglePlayPause);
    elements.playerNextBtn.addEventListener('click', nextTrack);
    elements.playerPrevBtn.addEventListener('click', previousTrack);
    elements.playerShuffleBtn.addEventListener('click', toggleShuffle);
    elements.playerRepeatBtn.addEventListener('click', toggleRepeat);
    elements.playerMuteBtn.addEventListener('click', toggleMute);

    // Volume Slider
    elements.playerVolumeSlider.addEventListener('input', (e) => {
      setVolume(parseFloat(e.target.value));
    });

    // Seekable Progress Bar Scrubbing
    elements.progressSlider.addEventListener('mousedown', () => { state.isSeeking = true; });
    elements.progressSlider.addEventListener('touchstart', () => { state.isSeeking = true; });

    elements.progressSlider.addEventListener('input', (e) => {
      elements.progressFill.style.width = `${e.target.value}%`;
      if (audio.duration) {
        elements.playerElapsed.textContent = formatDuration((parseFloat(e.target.value) / 100) * audio.duration);
      }
    });

    elements.progressSlider.addEventListener('change', (e) => {
      state.isSeeking = false;
      seekToPercent(parseFloat(e.target.value));
    });

    // Modals Open
    elements.btnOpenCreatePlaylistModal.addEventListener('click', () => {
      elements.inputPlaylistName.value = '';
      elements.modalCreatePlaylist.style.display = 'flex';
    });
    elements.btnCreatePlaylistHeader.addEventListener('click', () => {
      elements.inputPlaylistName.value = '';
      elements.modalCreatePlaylist.style.display = 'flex';
    });

    elements.btnOpenAddSongModal.addEventListener('click', () => {
      elements.formAddSong.reset();
      elements.modalAddSong.style.display = 'flex';
    });

    elements.btnOpenLoadPlaylistModal.addEventListener('click', () => {
      elements.inputLoadFilename.value = 'sample_tracks.csv';
      elements.modalLoadPlaylist.style.display = 'flex';
    });

    elements.btnConfirmAddToPlaylist.addEventListener('click', confirmAddTrackToPlaylist);

    // Modals Close
    document.querySelectorAll('[data-close]').forEach(btn => {
      btn.addEventListener('click', () => {
        const modalId = btn.dataset.close;
        const modal = document.getElementById(modalId);
        if (modal) modal.style.display = 'none';
      });
    });

    // Close on overlay backdrop click
    document.querySelectorAll('.modal-overlay').forEach(overlay => {
      overlay.addEventListener('click', (e) => {
        if (e.target === overlay) {
          overlay.style.display = 'none';
        }
      });
    });

    // Modal Form Submissions
    elements.formCreatePlaylist.addEventListener('submit', async (e) => {
      e.preventDefault();
      const name = elements.inputPlaylistName.value.trim();
      if (!name) return;

      try {
        const created = await apiFetch('/api/playlists', {
          method: 'POST',
          body: JSON.stringify({ name })
        });
        elements.modalCreatePlaylist.style.display = 'none';
        await fetchPlaylists();
        selectPlaylist(created.id, true);
        showToast(`Created playlist "${created.name}"!`, 'success');
      } catch (err) {
        showToast(err.message, 'warning');
      }
    });

    elements.formAddSong.addEventListener('submit', async (e) => {
      e.preventDefault();
      const payload = {
        id: parseInt(document.getElementById('inputSongId').value, 10),
        title: document.getElementById('inputSongTitle').value.trim(),
        artist: document.getElementById('inputSongArtist').value.trim(),
        genre: document.getElementById('inputSongGenre').value.trim(),
        duration: parseInt(document.getElementById('inputSongDuration').value, 10)
      };

      try {
        await apiFetch('/api/songs', {
          method: 'POST',
          body: JSON.stringify(payload)
        });
        elements.modalAddSong.style.display = 'none';
        await fetchSongs();
        renderGenrePills();
        renderLibraryTable();
        renderHomeView();
        showToast(`Track "${payload.title}" added to master library!`, 'success');
      } catch (err) {
        showToast(err.message, 'error');
      }
    });

    elements.formLoadPlaylist.addEventListener('submit', async (e) => {
      e.preventDefault();
      const filename = elements.inputLoadFilename.value.trim();
      if (!filename) return;

      try {
        const loaded = await apiFetch('/api/playlists/load', {
          method: 'POST',
          body: JSON.stringify({ filename })
        });
        elements.modalLoadPlaylist.style.display = 'none';
        await fetchPlaylists();
        selectPlaylist(loaded.id, true);
        showToast(`Loaded playlist "${loaded.name}" with ${loaded.trackCount} tracks!`, 'success');
      } catch (err) {
        showToast('Load failed: ' + err.message, 'error');
      }
    });

    // Retry connection button
    if (elements.btnRetryConnection) {
      elements.btnRetryConnection.addEventListener('click', () => {
        hideMainError();
        loadInitialData();
      });
    }
  }

  // ── Application Initialization ───────────────────────────────────────

  async function loadInitialData() {
    showLoading('Loading catalog from C++17 Synthesizer Engine...');
    hideMainError();
    try {
      await fetchSongs();
      await fetchPlaylists();
      renderGenrePills();
      renderLibraryTable();
      renderHomeView();

      // If playlists exist, select the first one by default using its actual ID
      if (state.playlists && state.playlists.length > 0) {
        const firstPl = state.playlists[0];
        state.selectedPlaylistId = firstPl.id;
        state.selectedPlaylistDetails = await apiFetch(`/api/playlists/${firstPl.id}`);
        renderSidebarPlaylists();
      } else {
        state.selectedPlaylistId = null;
        state.selectedPlaylistDetails = null;
        renderSidebarPlaylists();
      }
      hideLoading();
    } catch (err) {
      console.error('Initial data fetch failed:', err);
      hideLoading();
      showMainError('API Request Failed', `Unable to load data from C++ backend (${err.message}). Ensure the server is online at http://127.0.0.1:8080.`);
      setBackendStatus(false);
    }
  }

  window.addEventListener('DOMContentLoaded', () => {
    setupAudioEngine();
    setupUploadHandlers();
    setupKeyboardShortcuts();
    setupEventListeners();
    loadInitialData();
  });

})();
