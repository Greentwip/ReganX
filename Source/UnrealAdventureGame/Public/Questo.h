// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"

#include <iostream>
#include <vector>
#include <random>
#include <map>
#include <string>
#include <algorithm>  // For std::transform
#include <optional>

namespace QuestoUtil {

static std::string getLowerCase(const std::string& name) {
	std::string lowerCaseName = name;
	std::transform(lowerCaseName.begin(), lowerCaseName.end(), lowerCaseName.begin(), ::tolower);
	return lowerCaseName;
}

}

using CharacterId = std::string;
using GibberishId = std::string;

// Class representing a dialogue
class Dialog {
private:
	std::string content;
	std::map<GibberishId, std::vector<unsigned char>> gibberish;

public:
	Dialog(const std::string& content, const std::map<GibberishId,
		   std::vector<unsigned char>>& audio) : content(content), gibberish(audio) {
	}
	
	// Function to display the dialogue with the specified index
	std::string getString() const {
		return content;
	}
	
	// Function to display the dialogue with the specified index
	const std::vector<unsigned char> getGibberish() const {
		if (!gibberish.empty()) {
			
			static std::random_device rd;
			static std::mt19937 gen(rd());
			std::uniform_int_distribution<size_t> distribution(0, gibberish.size() - 1);
			size_t randomIndex = distribution(gen);

			auto it = std::next(std::begin(gibberish), randomIndex);
			
			return it->second;

		} else {
			return {};
		}
	}
	
	void setString(const std::string& dialog){
		content = dialog;
	}

	void appendGibberish(const GibberishId& gibberishId, const std::vector<unsigned char>& audio){
		gibberish[gibberishId] = audio;
	}

	void clearGibberish() {
		gibberish.clear();
	}
};

// Class representing a sub-stage
class SubStage {
private:
	std::string name;
	bool completed;
	std::map<CharacterId, Dialog> dialog;

public:
	SubStage(const std::string& inputName) : name(inputName), completed(false) {}
	
	// Function to complete the sub-stage
	void complete() {
		completed = true;
		std::cout << "Sub-Stage completed!" << std::endl;
	}
	
	bool isComplete() {
		return completed;
	}
	
	// Function to get the Dialog for a specific character
	bool getDialog(CharacterId character, Dialog** outDialog) {
		auto it = std::find_if(dialog.begin(), dialog.end(),
							   [character](const auto& pair) { return pair.first == character; });
		
		if (it != dialog.end()) {
			*outDialog = &it->second;
		} else {
			auto result = dialog.emplace(character, Dialog("", {}));

			*outDialog = &result.first->second;
		}
		
		return true;
	}
	
	void clearGibberishForCharacter(CharacterId character) {
		auto it = dialog.find(character);
		if (it != dialog.end()) {
			it->second.clearGibberish();
		}
	}

	void appendGibberishForCharacter(CharacterId character, GibberishId gibberishId, const std::vector<unsigned char>& audio) {
		auto it = dialog.find(character);
		if (it != dialog.end()) {
			it->second.appendGibberish(gibberishId, audio);
		}
	}

	void setDialogString(CharacterId character, const std::string& content) {
		auto it = dialog.find(character);
		if (it != dialog.end()) {
			it->second.setString(content);
		} else {
			dialog.insert({character, Dialog(content, {})});
		}
	}
	
	std::string getLowerCaseName() const {
		std::string lowerCaseName = name;
		std::transform(lowerCaseName.begin(), lowerCaseName.end(), lowerCaseName.begin(), ::tolower);
		return lowerCaseName;
	}
};

// Class representing a stage
class Stage {
private:
	std::string name;  // Added a name for the quest
	
	std::vector<SubStage> subStages;
	bool completed;
	
public:
	Stage(const std::string& inputName, const std::vector<SubStage>& inputSubStages) : name(inputName), subStages(inputSubStages), completed(false) {}
	
	bool isComplete() {
		return areAllStagesCompleted();
	}
	
	// Function to complete the current sub-stage
	void completeSubStage(const std::string& subStageName) {
		if(subStages.empty()){
			std::cout << "There are no substages for this stage!" << std::endl;
		} else {
			auto substage = getSubStage(subStageName);
			
			if(substage != std::nullopt){
				getSubStage(subStageName).value().get().complete();
			}
		}
	}
	
	bool areAllStagesCompleted() {
		if(subStages.empty()){
			return false;
		}
		
		if (!completed) {
			for (auto& subStage : subStages) {
				if (!subStage.isComplete()) {
					return false;
				}
			}
			
			completed = true;
		}
		return completed;
	}
	
	bool createEmptySubStage(const std::string& subStageName)
	{
		// Check if the stageName already exists in a case-insensitive manner
		std::string lowerCaseStageName = QuestoUtil::getLowerCase(subStageName);
		auto it = std::find_if(subStages.begin(), subStages.end(),
							   [lowerCaseStageName](const SubStage& subStage)
							   {
			return subStage.getLowerCaseName() == lowerCaseStageName;
		});
		
		if (it == subStages.end())
		{
			subStages.push_back(SubStage(lowerCaseStageName));
			return true;  // Indicate success
		}
		else
		{
			// Stage with the given name already exists
			return false;  // Indicate failure
		}
	}
	
	bool findSubStage(const std::string& subStageName)
	{
		// Check if the stageName already exists in a case-insensitive manner
		std::string lowerCaseStageName = QuestoUtil::getLowerCase(subStageName);
		auto it = std::find_if(subStages.begin(), subStages.end(),
							   [lowerCaseStageName](const SubStage& subStage)
							   {
			return subStage.getLowerCaseName() == lowerCaseStageName;
		});
		
		if (it != subStages.end())
		{
			return true;  // Indicate success
		}
		else
		{
			// Stage with the given name already exists
			return false;  // Indicate failure
		}
	}
	
	// Function to get the vector of Dialog for a specific sub-stage index
	SubStage& getSubStage(int subStageIndex) {
		if (subStageIndex >= 0 && subStageIndex < subStages.size()) {
			return subStages[subStageIndex];
		} else {
			throw std::out_of_range("Sub-stage index out of range");
		}
	}
	
	std::optional<std::reference_wrapper<SubStage>> getSubStage(const std::string& subStageName) {
		std::string lowerCaseSubStageName = QuestoUtil::getLowerCase(subStageName);

		// Find the quest based on the lowercase name
		auto it = std::find_if(subStages.begin(), subStages.end(),
							   [&lowerCaseSubStageName](const SubStage& s) { return s.getLowerCaseName() == lowerCaseSubStageName; });
		
		if (it != subStages.end()) {
			return *it;
		} else {
			std::cout << "SubStage not found" << std::endl;
			return std::nullopt;
		}
	}
	
	std::optional<std::reference_wrapper<SubStage>> getActiveSubStage() {
		if(subStages.empty()){
			return std::nullopt;
		} else {
			
			for (auto& subStage : subStages) {
				if (!subStage.isComplete()) {
					return subStage;
				}
			}
		}
		
		return subStages.back();;
	}
	std::string getLowerCaseName() const {
		std::string lowerCaseName = name;
		std::transform(lowerCaseName.begin(), lowerCaseName.end(), lowerCaseName.begin(), ::tolower);
		return lowerCaseName;
	}
};

// Class representing a quest
class Quest {
private:
	std::string name;  // Added a name for the quest
	std::vector<Stage> stages;
	
public:
	Quest(const std::string& inputName, const std::vector<Stage>& inputStages) : name(inputName), stages(inputStages) {}
	
	bool createEmptyStage(const std::string& stageName)
	{
		// Check if the stageName already exists in a case-insensitive manner
		std::string lowerCaseStageName = QuestoUtil::getLowerCase(stageName);
		auto it = std::find_if(stages.begin(), stages.end(),
							   [lowerCaseStageName](const Stage& stage)
							   {
			return stage.getLowerCaseName() == lowerCaseStageName;
		});
		
		if (it == stages.end())
		{
			// Stage with the given name doesn't exist, add a new one
			stages.push_back(Stage({lowerCaseStageName, {}}));
			return true;  // Indicate success
		}
		else
		{
			// Stage with the given name already exists
			return false;  // Indicate failure
		}
	}
	
	bool findStage(const std::string& stageName)
	{
		// Check if the stageName already exists in a case-insensitive manner
		std::string lowerCaseStageName = QuestoUtil::getLowerCase(stageName);
		auto it = std::find_if(stages.begin(), stages.end(),
							   [lowerCaseStageName](const Stage& stage)
							   {
			return stage.getLowerCaseName() == lowerCaseStageName;
		});
		
		if (it != stages.end())
		{
			return true;  // Indicate success
		}
		else
		{
			// Stage with the given name already exists
			return false;  // Indicate failure
		}
	}

	// Function to check if all stages are completed
	bool areAllStagesCompleted() {
		for (auto& stage : stages) {
			if (!stage.isComplete()) {
				return false;
			}
		}
		return true;
	}
	
	// Function to get a quest from the system
	std::optional<std::reference_wrapper<Stage>> getStage(const std::string& stageName) {
		
		std::string lowerCaseStageName = QuestoUtil::getLowerCase(stageName);

		// Find the quest based on the lowercase name
		auto it = std::find_if(stages.begin(), stages.end(),
							   [&lowerCaseStageName](const Stage& s) { return s.getLowerCaseName() == lowerCaseStageName; });
		
		if (it != stages.end()) {
			return *it;
		} else {
			std::cout << "Stage not found" << std::endl;
			return std::nullopt;
		}
	}
	
	std::optional<std::reference_wrapper<Stage>> getActiveStage() {
		if(stages.empty()){
			return std::nullopt;
		} else {
			for (auto& stage : stages) {
				if (!stage.isComplete()) {
					return stage;
				}
			}
		}
		
		return stages.back();
	}

	
	// Function to get the quest name in lowercase
	std::string getLowerCaseName() const {
		std::string lowerCaseName = name;
		std::transform(lowerCaseName.begin(), lowerCaseName.end(), lowerCaseName.begin(), ::tolower);
		return lowerCaseName;
	}
};

// Class representing a quest system
class QuestSystem {
private:
	std::vector<Quest> quests;
	std::string activeQuest;
	
public:
	QuestSystem() {
		std::vector<SubStage> subStages1 = {{"main"}};
		
		std::vector<Stage> predefinedStages = {{"main", subStages1}};
		
		// Creating quests
		Quest mainQuest("main", predefinedStages);
		
		addQuest(mainQuest);
		
		activeQuest = "main";
	}
	
	bool createEmptyQuest(const std::string& questName){
		std::string lowerCaseName = QuestoUtil::getLowerCase(questName);

		if(getQuest(lowerCaseName) != std::nullopt){
			return false;
		} else {
			addQuest({lowerCaseName, {}});
			return true;
		}
	}
	
	bool findQuest(const std::string& questName){
		std::string lowerCaseName = QuestoUtil::getLowerCase(questName);

		if(getQuest(lowerCaseName) != std::nullopt){
			return true;
		} else {
			return false;
		}
	}
	
	void setActiveQuest(const std::string& questName){
		std::string lowerCaseName = QuestoUtil::getLowerCase(questName);
		
		if(getQuest(lowerCaseName) != std::nullopt){
			activeQuest = questName;
		} 
	}
	
	// Function to get a quest from the system
	std::optional<std::reference_wrapper<Quest>> getActiveQuest() {
		std::string questName = activeQuest;
		
		std::string lowerCaseName = QuestoUtil::getLowerCase(questName);
		
		// Find the quest based on the lowercase name
		auto it = std::find_if(quests.begin(), quests.end(),
							   [&lowerCaseName](const Quest& q) { return q.getLowerCaseName() == lowerCaseName; });
		
		if (it != quests.end()) {
			return *it;
		} else {
			std::cout << "Quest not found" << std::endl;
			return std::nullopt;
		}
	}
	
	// Function to get a quest from the system
	std::optional<std::reference_wrapper<Quest>> getQuest(const std::string& questName) {
		std::string lowerCaseName = QuestoUtil::getLowerCase(questName);

		// Find the quest based on the lowercase name
		auto it = std::find_if(quests.begin(), quests.end(),
							   [&lowerCaseName](const Quest& q) { return q.getLowerCaseName() == lowerCaseName; });
		
		if (it != quests.end()) {
			return *it;
		} else {
			std::cout << "Quest not found" << std::endl;
			return std::nullopt;
		}
	}
	
private:
	// Function to add a quest to the system
	void addQuest(const Quest& quest) {
		quests.push_back(quest);
	}
};
