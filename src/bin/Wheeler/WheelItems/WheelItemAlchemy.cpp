#include "bin/Rendering/Drawer.h"
#include "bin/Utilities/Utils.h"
#include "WheelItemAlchemy.h"

WheelItemAlchemy::WheelItemAlchemy(RE::AlchemyItem* a_alchemyItem)
{
	this->_alchemyItem = a_alchemyItem;
	Texture::icon_image_type iconType = Texture::icon_image_type::food;
	if (_alchemyItem->data.flags.any(RE::AlchemyItem::AlchemyFlag::kFoodItem)  // food icon
		|| _alchemyItem->HasKeywordString("VendorItemFood")) {
		iconType = Texture::icon_image_type::food;
		this->_alchemyItemType = WheelItemAlchemyType::kFood;
	} else if (_alchemyItem->data.flags.any(RE::AlchemyItem::AlchemyFlag::kPoison)  // poison icon
			   || _alchemyItem->HasKeywordString("VendorItemPoison")) {
		iconType = Texture::icon_image_type::poison_default;
		this->_alchemyItemType = WheelItemAlchemyType::kPoison;
	} else if (_alchemyItem->data.flags.any(RE::AlchemyItem::AlchemyFlag::kMedicine)  //potion icon
			   || _alchemyItem->HasKeywordString("VendorItemPotion")) {
		// https://github.com/mlthelama/LamasTinyHUD/blob/main/src/handle/page_handle.cpp
		iconType = Texture::icon_image_type::potion_default;
		this->_alchemyItemType = WheelItemAlchemyType::kPotion;
		const RE::EffectSetting* effect = _alchemyItem->GetCostliestEffectItem()->baseEffect;
		RE::ActorValue actorValue = effect->GetMagickSkill();
		if (actorValue == RE::ActorValue::kNone) {
			actorValue = effect->data.primaryAV;
		}
		switch (actorValue) {
		case RE::ActorValue::kHealth:
		case RE::ActorValue::kHealRateMult:
		case RE::ActorValue::kHealRate:
			iconType = Texture::icon_image_type::potion_health;
			break;
		case RE::ActorValue::kStamina:
		case RE::ActorValue::kStaminaRateMult:
		case RE::ActorValue::KStaminaRate:
			iconType = Texture::icon_image_type::potion_stamina;
			break;
		case RE::ActorValue::kMagicka:
		case RE::ActorValue::kMagickaRateMult:
		case RE::ActorValue::kMagickaRate:
			iconType = Texture::icon_image_type::potion_magicka;
			break;
		case RE::ActorValue::kResistFire:
			iconType = Texture::icon_image_type::potion_fire_resist;
			break;
		case RE::ActorValue::kResistShock:
			iconType = Texture::icon_image_type::potion_shock_resist;
			break;
		case RE::ActorValue::kResistFrost:
			iconType = Texture::icon_image_type::potion_frost_resist;
			break;
		case RE::ActorValue::kResistMagic:
			iconType = Texture::icon_image_type::potion_magic_resist;
			break;
		default:
			iconType = Texture::icon_image_type::potion_default;
		}
	}
	this->_texture = Texture::GetIconImage(iconType, this->_alchemyItem);
	Utils::Magic::GetMagicItemDescription(_alchemyItem, this->_description);
}

void WheelItemAlchemy::DrawSlot(ImVec2 a_center, bool a_hovered, RE::TESObjectREFR::InventoryItemMap& a_imap, DrawArgs a_drawArgs)
{
	{
		using namespace Config::Styling::Item::Slot;
		int itemCount = a_imap.contains(this->_alchemyItem) ? a_imap.find(this->_alchemyItem)->second.first : 0;
		Drawer::draw_text(a_center.x + Text::OffsetX, a_center.y + Text::OffsetY,
			fmt::format("{} ({})", _alchemyItem->GetName(), itemCount).data(), C_SKYRIMWHITE, Text::Size, a_drawArgs);
		Drawer::draw_texture(_texture.texture,
			ImVec2(a_center.x, a_center.y),
			Config::Styling::Item::Slot::Texture::OffsetX,
			Config::Styling::Item::Slot::Texture::OffsetY,
			ImVec2(_texture.width * Config::Styling::Item::Slot::Texture::Scale, _texture.height * Config::Styling::Item::Slot::Texture::Scale),
			C_SKYRIMWHITE, a_drawArgs);
	}
}

void WheelItemAlchemy::DrawHighlight(ImVec2 a_center, RE::TESObjectREFR::InventoryItemMap& a_imap, DrawArgs a_drawArgs)
{
	{
		using namespace Config::Styling::Item::Highlight;
		Drawer::draw_text(a_center.x + Text::OffsetX, a_center.y + Text::OffsetY,
			_alchemyItem->GetName(), C_SKYRIMWHITE, Text::Size, a_drawArgs);
	}

	Drawer::draw_texture(_texture.texture,
		ImVec2(a_center.x, a_center.y),
		Config::Styling::Item::Highlight::Texture::OffsetX,
		Config::Styling::Item::Highlight::Texture::OffsetY,
		ImVec2(_texture.width * Config::Styling::Item::Highlight::Texture::Scale, _texture.height * Config::Styling::Item::Highlight::Texture::Scale),
		C_SKYRIMWHITE, a_drawArgs);

	if (!this->_description.empty()) {
		Drawer::draw_text_block(a_center.x + Config::Styling::Item::Highlight::Desc::OffsetX, a_center.y + Config::Styling::Item::Highlight::Desc::OffsetY,
			this->_description, C_SKYRIMWHITE, Config::Styling::Item::Highlight::Desc::Size, Config::Styling::Item::Highlight::Desc::LineSpacing, Config::Styling::Item::Highlight::Desc::LineLength, a_drawArgs);
	}
}

bool WheelItemAlchemy::IsActive(RE::TESObjectREFR::InventoryItemMap& a_inv)
{
	return false;
}

bool WheelItemAlchemy::IsAvailable(RE::TESObjectREFR::InventoryItemMap& a_inv)
{
	return a_inv.contains(this->_alchemyItem);
}

void WheelItemAlchemy::ActivateItemPrimary()
{
	switch (this->_alchemyItemType) {
	case WheelItemAlchemyType::kPotion:
	case WheelItemAlchemyType::kFood:
		this->consume();
	}
}

void WheelItemAlchemy::ActivateItemSecondary()
{
	switch (this->_alchemyItemType) {
	case WheelItemAlchemyType::kPotion:
	case WheelItemAlchemyType::kFood:
		this->consume();
	}
}

void WheelItemAlchemy::ActivateItemSpecial()
{
	return;
}

void WheelItemAlchemy::SerializeIntoJsonObj(nlohmann::json& a_json)
{
	a_json["type"] = WheelItemAlchemy::ITEM_TYPE_STR;
	a_json["formID"] = this->_alchemyItem->GetFormID();
}

void WheelItemAlchemy::consume()
{
	if (this->_alchemyItemType != WheelItemAlchemyType::kFood && this->_alchemyItemType != WheelItemAlchemyType::kPotion) {
		throw std::exception("WheelItemAlchemy::consume : Item is not a food or potion");
	}
	RE::PlayerCharacter* pc = RE::PlayerCharacter::GetSingleton();
	if (!pc) {
		return;
	}
	RE::ActorEquipManager::GetSingleton()->EquipObject(pc, this->_alchemyItem);
}
