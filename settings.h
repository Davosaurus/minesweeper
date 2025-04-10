#include "ms.h"

class Settings {
	private:
		unordered_map<string, string> settings = {
			{"evaluator", "safeStart"},
			{"containerType", "fragmented"},
			{"pixelDisplayThreshold", "3"},
			{"playerName", ""}
		};
		
		const vector<string> evaluatorNames = {
			"random",
			"safeStart",
			"safeStartPlus",
			"noGuess"
		};
		
		const vector<string> containerTypeNames = {
			"fragmented",
			"random",
			"ordered"
		};
	public:
		/**
		 * Create a new settings object, replacing default values with any from config file if present
		 */
		Settings() {
			//Override settings that are present in config file
			bool openFile = false;
			if(openFile) {
				//do read
				//close file
			}
		}
		
		/**
		 * Save this settings object to config file
		 */
		void save() const {
			
		}
		
		const function<bool(const Field& field, const short& row, const short& col)>* getEvaluator() {
			return &field::evaluators::evaluator[settings["evaluator"]];
		}
		
		const short getEvaluatorOrdinal() {
			return distance(evaluatorNames.begin(), find(evaluatorNames.begin(), evaluatorNames.end(), settings["evaluator"])) + 1;
		}
		
		void setEvaluatorByOrdinal(const short& ordinal) {
			settings["evaluator"] = evaluatorNames[ordinal - 1];
		}
		
		const string getContainerType() {
			return settings["containerType"];
		}
		
		const short getContainerTypeOrdinal() {
			return distance(containerTypeNames.begin(), find(containerTypeNames.begin(), containerTypeNames.end(), settings["containerType"])) + 1;
		}
		
		void setContainerTypeByOrdinal(const short& ordinal) {
			settings["containerType"] = containerTypeNames[ordinal - 1];
		}
		
		const string getPlayerName() {
			return settings["playerName"];
		}
		
		void setPlayerName(const string& name) {
			settings["playerName"] = name;
		}
		
		const short getPixelDisplayThreshold() {
			return stoi(settings["pixelDisplayThreshold"]);
		}
		
		void setPixelDisplayThreshold(const short& threshold) {
			settings["pixelDisplayThreshold"] = to_string(threshold);
		}
};