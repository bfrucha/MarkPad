//
//  macdesktop: functions for managing the MacOSX desktop.
//  (C) Eric Lecolinet 2017 - www.telecom-paristech.fr/~elc
//

/** @file
 * Class and functions for managing the MacOSX desktop.
 * @author Eric Lecolinet 2017 - https://www.telecom-paristech.fr/~elc
 */

#ifndef ccuty_macdesktop
#define ccuty_macdesktop

#include <string>

namespace ccuty {
  
  /** Class for managing the MacOSX desktop.
   */
  class MacDesktop {
  public:
    static MacDesktop instance;
    
    /// calls the C `system()` function.
    int system(const std::string& command);
    
    /// calls the C popen() function and returns the result.
    int system(const std::string& command, std::string& result);
  
    /// activates an application.
    void activateApp(const std::string& appName);
    
    /// returns the path of the front-most application.
    bool getFrontAppPath(std::string& str);
    
    /// returns the name of the front-most application.
    bool getFrontAppName(std::string& str);
    
    /// returns the localized name of the front-most application.
    bool getFrontAppLocName(std::string& str);

    /// opens a file or an URL.
    /// path must be the pathname of the file.
    /// applications are executed.
    /// URLs must start with a valid prefix (http:, https:, file:)
    void open(const std::string& path);
    
    /// opens an URL using the default web browser.
    /// the prefix (http:, https:, file:) may be missing.
    void openUrl(const std::string& url);

    /// opens a file.
    /// path must be the pathname of the file.
    /// Apple and Unix scripts are opened in editor.
    void openFile(const std::string& path);

    /// opens an application.
    /// path can be the name or the pathname of the application.
    /// Apple and Unix scripts are executed as applications.
    void openApp(const std::string& path);
    
    /// opens an application with an argument.
    void openApp(const std::string& path, const std::string& arg);
    
    /// hides an application.
    void hideApp(const std::string& path);
    
    /// sends an AppleScript command to an application.
    void tellApp(const std::string& path, const std::string& command, bool inBackground = true);
    
    /// sends an AppleScript command to an application and return the result.
    void tellApp(const std::string& path, const std::string& command, std::string& result);
    
    /// reactivates the screen if it's off.
    void wakeScreen();
    
    /// increasees or decrease current volume depending on value sign (exple: -10 or +10).
    void changeVolume(int value, bool displayNotification = false);
    
    /// sets the volume (value must be between 0 and 100).
    void setVolume(unsigned int value, bool displayNotification = false);
    
    /// aternately mute/unmutes the volume.
    void muteVolume();
    
    /// beeps the speaker.
    void beep(int seconds);
    
    /// says this message.
    void say(const std::string& message);
    
    /// aternately play/pause iTunes.
    void playPause();
    
    /// Plays one or several albums.
    /** Either _album_ or _artist_ may be empty, resp. meaning: play any album from
     *  this artist or any album with this name
     */
    bool playAlbum(const std::string& album, const std::string& artist);
    
    /// Plays tracks in a playlist.
    /** Plays all tracks if _track_ is empty.
     */
    bool playPlaylist(const std::string& playlist, const std::string& track = "");
    
    /// returns what is currently played or selected in iTunes.
    /** returns what is being played if _isPlaying_ is true and what is currently
     *  seclected if _isPlaying_ is false.
     */
    bool getCurrentTrack(std::string& album, std::string& artist, std::string& track,
                         bool isPlaying = true);
    
    /// aternately show/hides the Dock.
    void showHideDock();
    
    /// returns the width and height of the main screen.
    static void getMainScreenSize(size_t& width, size_t& height);
    
    /** @brief hide/unhides the mouse cursor.
     * hides the mouse if the argument is true, unhide otherwise.
     *  @note this action only takes effect on the application that calls this function,
     *  its does no hide the cursor if another applications is selected.
     */
    static void hideMouse(bool state);
    
    /// Mouse buttons.
    enum MouseButton {LeftButton=0, RightButton=1, CenterButton=3};

    /// gets mouse location (in screen coordinates).
    static void getMouse(double& x, double& y);
    
    /// generates a mouse down (_button_ is one of MacDesktop::MouseButton).
    static void pressMouse(double x, double y, unsigned int mouseButton);
    
    /// generates a mouse up (_button_ is one of MacDesktop::MouseButton.
    static void releaseMouse(double x, double y, unsigned int mouseButton);
    
    /// drags the mouse to this location (_button_ is one of MacDesktop::MouseButton).
    static void dragMouse(double x, double y, unsigned int mouseButton);
    
    /// moves the mouse to this location.
    static void moveMouse(double x, double y);
    
    /// Key modifiers.
    enum Modifiers {Shift=1<<0, Control=1<<1, Alt=1<<2, Command=1<<3};
    
    /// Key event types.
    enum KeyType {KeyDown=1<<0, KeyUp=1<<1, KeyType=(KeyDown|KeyUp)};

    /** @brief generates a keyStroke, possibly with modifiers.
     * _keystroke_ must start by an AppleScript keystroke possibily followed by modifiers.
     * _modifiers_ is 0 or an ORred combination of MacDesktop::Alt, Control, Command, Shift.
     * _modifiers_ should be 0 if modifiers are already present in _keystroke_
     *
     * _keystroke_ examples:
     *  - "tab"
     *  - "\"toto\""
     *  - "\"q\" using command down
     *  - "\"f\" using {control down, command down}"
     */
    void sendKeyStroke(const std::string& keystroke);
    
    /** @brief generates a keyCode, possibly with modifiers.
     * _keycode_ is a virtual key code.
     * _modifiers_ is 0 or an ORred combination of MacDesktop::Alt, Control, Command, Shift.
     * _keytype_ should either be MacDesktop::KeyDown, KeyUp or KeyType (the default)
     * @note Key codes are not localized but correspond to key locations on a standard US keyboard.
     */
    static void sendKeycode(uint32_t keycode, uint8_t modifiers=0, uint8_t keytype = KeyType);
    
    /// generates a LEFT key down/key up.
    static void sendLeftKey();
    
    /// generates a RIGHT key down/key up.
    static void sendRightKey();
    
    /// generates a UP key down/key up.
    static void sendUpKey();
    
    /// generates a DOWN key down/key up.
    static void sendDownKey();
    
    /// generates a TAB key down/key up.
    static void sendTabKey();
    
    /// generates a BACKTAB key down/key up.
    static void sendBacktabKey();
    
    /// generates a SPACE key down/key up.
    static void sendSpaceKey();
    
    /// generates an RETURN key down/key up.
    static void sendReturnKey();
    
    /// generates an DELETE key down/key up.
    static void sendDeleteKey();
    
    /// generates an ESCAPE key down/key up.
    static void sendEscapeKey();
  };
}

#endif
