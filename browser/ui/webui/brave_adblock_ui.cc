/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/ui/webui/brave_adblock_ui.h"

#include "brave/browser/brave_browser_process_impl.h"
#include "brave/common/pref_names.h"
#include "brave/common/webui_url_constants.h"
#include "brave/components/brave_adblock/resources/grit/brave_adblock_generated_map.h"
#include "brave/components/brave_shields/browser/ad_block_custom_filters_service.h"
#include "brave/components/brave_shields/browser/ad_block_regional_service.h"
#include "chrome/browser/profiles/profile.h"
#include "components/grit/brave_components_resources.h"
#include "components/prefs/pref_change_registrar.h"
#include "components/prefs/pref_service.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/render_view_host.h"
#include "content/public/browser/web_ui_data_source.h"
#include "content/public/browser/web_ui_message_handler.h"

namespace {

class AdblockDOMHandler : public content::WebUIMessageHandler {
 public:
  AdblockDOMHandler();
  ~AdblockDOMHandler() override;

  // WebUIMessageHandler implementation.
  void RegisterMessages() override;

 private:
  void HandleGetCustomFilters(const base::ListValue* args);
  void HandleUpdateCustomFilters(const base::ListValue* args);

  DISALLOW_COPY_AND_ASSIGN(AdblockDOMHandler);
};

AdblockDOMHandler::AdblockDOMHandler() {}

AdblockDOMHandler::~AdblockDOMHandler() {}

void AdblockDOMHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback(
      "brave_adblock.getCustomFilters",
      base::BindRepeating(&AdblockDOMHandler::HandleGetCustomFilters,
                          base::Unretained(this)));
  web_ui()->RegisterMessageCallback(
      "brave_adblock.updateCustomFilters",
      base::BindRepeating(&AdblockDOMHandler::HandleUpdateCustomFilters,
                          base::Unretained(this)));
}

void AdblockDOMHandler::HandleGetCustomFilters(const base::ListValue* args) {
  DCHECK_EQ(args->GetSize(), 0U);
  const std::string custom_filters =
      g_brave_browser_process->ad_block_custom_filters_service()
          ->GetCustomFilters();
  if (!web_ui()->CanCallJavascript())
    return;
  web_ui()->CallJavascriptFunctionUnsafe("brave_adblock.onGetCustomFilters",
                                         base::Value(custom_filters));
}

void AdblockDOMHandler::HandleUpdateCustomFilters(const base::ListValue* args) {
  DCHECK_EQ(args->GetSize(), 1U);
  std::string custom_filters;
  if (!args->GetString(0, &custom_filters))
    return;

  g_brave_browser_process->ad_block_custom_filters_service()
      ->UpdateCustomFilters(custom_filters);
}

}  // namespace

BraveAdblockUI::BraveAdblockUI(content::WebUI* web_ui, const std::string& name)
    : BasicUI(web_ui, name, kBraveAdblockGenerated,
        kBraveAdblockGeneratedSize, IDR_BRAVE_ADBLOCK_HTML) {
  Profile* profile = Profile::FromWebUI(web_ui);
  PrefService* prefs = profile->GetPrefs();
  pref_change_registrar_ = std::make_unique<PrefChangeRegistrar>();
  pref_change_registrar_->Init(prefs);
  pref_change_registrar_->Add(kAdsBlocked,
    base::Bind(&BraveAdblockUI::OnPreferenceChanged, base::Unretained(this)));
  web_ui->AddMessageHandler(std::make_unique<AdblockDOMHandler>());
}

BraveAdblockUI::~BraveAdblockUI() {
}

void BraveAdblockUI::CustomizeWebUIProperties(
    content::RenderViewHost* render_view_host) {
  DCHECK(IsSafeToSetWebUIProperties());

  Profile* profile = Profile::FromWebUI(web_ui());
  PrefService* prefs = profile->GetPrefs();
  if (render_view_host) {
    render_view_host->SetWebUIProperty(
        "adsBlockedStat", std::to_string(prefs->GetUint64(kAdsBlocked)));
    render_view_host->SetWebUIProperty(
        "regionalAdBlockEnabled",
        std::to_string(g_brave_browser_process->ad_block_regional_service()
                           ->IsInitialized()));
    render_view_host->SetWebUIProperty(
        "regionalAdBlockTitle",
        g_brave_browser_process->ad_block_regional_service()->GetTitle());
  }
}

void BraveAdblockUI::UpdateWebUIProperties() {
  if (IsSafeToSetWebUIProperties()) {
    CustomizeWebUIProperties(GetRenderViewHost());
    web_ui()->CallJavascriptFunctionUnsafe("brave_adblock.statsUpdated");
  }
}

void BraveAdblockUI::OnPreferenceChanged() {
  UpdateWebUIProperties();
}
