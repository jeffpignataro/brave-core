/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_EXTENSIONS_BRAVE_COMPONENT_LOADER_H_
#define BRAVE_BROWSER_EXTENSIONS_BRAVE_COMPONENT_LOADER_H_

#include "base/files/file_path.h"
#include "chrome/browser/extensions/component_loader.h"
#include "components/prefs/pref_change_registrar.h"

class BravePDFExtensionTest;

namespace extensions {

// For registering, loading, and unloading component extensions.
class BraveComponentLoader : public ComponentLoader {
 public:
  BraveComponentLoader(ExtensionServiceInterface* extension_service,
                  PrefService* prefs,
                  PrefService* local_state,
                  Profile* browser_context);
  ~BraveComponentLoader() override;

  // Adds the default component extensions. If |skip_session_components|
  // the loader will skip loading component extensions that weren't supposed to
  // be loaded unless we are in signed user session (ChromeOS). For all other
  // platforms this |skip_session_components| is expected to be unset.
  void AddDefaultComponentExtensions(bool skip_session_components) override;
  void OnComponentRegistered(std::string extension_id);
  void OnComponentReady(std::string extension_id,
    bool allow_file_access,
    const base::FilePath& install_dir,
    const std::string& manifest);
  void AddExtension(const std::string& id,
      const std::string& name, const std::string& public_key);
  // ForceAddHangoutServicesExtension ignores whether or not a preference for
  // hangouts is set.  If the buildflag is not set, it won't add though.
  void ForceAddHangoutServicesExtension();
 
  static bool IsPdfjsDisabled();

 private:
  void AddHangoutServicesExtension() override;
  friend class ::BravePDFExtensionTest;
  void ObserveOpenPdfExternallySetting();
  // Callback for changes to the AlwaysOpenPdfExternally setting.
  void UpdatePdfExtension(const std::string& pref_name);

  struct TestingCallbacks {
    enum PdfExtensionAction {
      NONE,
      WILL_ADD,
      WILL_REMOVE,
    };
    virtual void OnPdfExtensionAction(PdfExtensionAction action) = 0;
  };

  void set_testing_callbacks(TestingCallbacks* testing_callbacks);

  Profile* profile_;
  PrefService* profile_prefs_;
  PrefChangeRegistrar registrar_;
  TestingCallbacks* testing_callbacks_;
  DISALLOW_COPY_AND_ASSIGN(BraveComponentLoader);
};

}  // namespace extensions

#endif  // BRAVE_BROWSER_EXTENSIONS_BRAVE_COMPONENT_LOADER_H_
