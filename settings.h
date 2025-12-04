#include "ms.h"
#include <chrono>
#include <fstream>
#include <set>

//Technically also stores high scores which isn't strictly a setting but it is still saved to disk in the same fashion

class HighScore {
	public:
		short cols;
		short rows;
		int mines;
		ULONGLONG elapsedTime;
		string playerName;
		
		HighScore(const Minefield& game, const string& desiredPlayerName) {
			cols = game.getCols();
			rows = game.getRows();
			mines = game.getMines();
			elapsedTime = game.getElapsedTime();
			playerName = stringUtils::uppercase(desiredPlayerName);
		}
		
		HighScore(const string& saveData) {
			short start = 0;
			short end = saveData.find(character::DELIMITER, start);
			cols = stoi(saveData.substr(start, end - start));
			
			start = end + 1;
			end = saveData.find(character::DELIMITER, start);
			rows = stoi(saveData.substr(start, end - start));
			
			start = end + 1;
			end = saveData.find(character::DELIMITER, start);
			mines = stoi(saveData.substr(start, end - start));
			
			start = end + 1;
			end = saveData.find(character::DELIMITER, start);
			elapsedTime = stoull(saveData.substr(start, end - start));
			
			start = end + 1;
			end = saveData.find(character::DELIMITER, start);
			playerName = saveData.substr(start, end - start);
			
			short finalScoreDelimiter = playerName.find(character::DATA_END);
			playerName = playerName.substr(0, finalScoreDelimiter);
		}
		
		vector<string> getRawData() const {
			vector<string> data;
			data.push_back(to_string(cols));
			data.push_back(to_string(rows));
			data.push_back(to_string(mines));
			data.push_back(to_string(elapsedTime));
			data.push_back(playerName);
			return data;
		}
		
		FlexibleString getDisplayString(const short& totalWidth) const {
			chrono::duration<ULONGLONG, milli> durationMillis(elapsedTime);
			short wholeHours = durationMillis / 1h;
			short wholeMins = durationMillis % 1h / 1min;
			short wholeSeconds = durationMillis % 1min / 1s;
			short wholeMillis = durationMillis % 1s / 1ms;
			
			string gameTimeString = character::STRING_CLOCK;
			if(wholeHours > 0) {
				gameTimeString += to_string(wholeHours) + "h ";
			}
			if(wholeHours > 0 || wholeMins > 0) {
				gameTimeString += to_string(wholeMins) + "m ";
			}
			gameTimeString += to_string(wholeSeconds);
			if(durationMillis < 1min) {
				char milliString[4];
				sprintf(milliString, "%03d", wholeMillis);
				gameTimeString += "." + string(milliString);
			}
			gameTimeString += "s";
			
			FlexibleString result;
			result.addText(to_string(cols) + "x" + to_string(rows), 9);
			result.addText(character::STRING_MINE + to_string(mines), 9);
			result.addText(gameTimeString);
			result.addText(playerName, 3);
			return result;
		}
		
		string serialize() const {
			string result;
			auto data = getRawData();
			for(auto member = data.begin();; member++) {
				result += *member;
				if(member + 1 != data.end()) {
					result += character::DELIMITER;
				}
				else {
					result += character::DATA_END;
					break;
				}
			}
			return result;
		}
};

/**
 * Compares 2 HighScore objects in a strict weak ordering.
 * Harder fields should be first.
 */
struct HighScoreComparator {
	bool operator()(HighScore hsA, HighScore hsB) const {
		const double hsASize = hsA.cols * hsA.rows;
		const double hsBSize = hsB.cols * hsB.rows;

		const double hsADensity = hsA.mines / hsASize;
		const double hsBDensity = hsB.mines / hsBSize;
		
		return hsADensity != hsBDensity
				? hsADensity > hsBDensity //higher mine density
				: hsA.mines != hsB.mines
						? hsA.mines > hsB.mines //more mines
						: hsASize < hsBSize; //smaller field size
	}
};

class Settings {
	private:
		unordered_map<string, string> settings = {
			{"evaluator", "safeStartPlus"},
			{"containerType", "ordered"},
			{"pixelDisplayThreshold", "4"},
			{"playerName", ""},
			{"solverModeDelay", "-1"},
			{"highScores", ""}
		};
		
		set<HighScore, HighScoreComparator> highScores = set<HighScore, HighScoreComparator>();
		
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
			ifstream saveFile("ms.dat");
			
			//Read in all key-value pair settings
			string line;
			while(getline(saveFile, line)) {
				int separator = line.find(character::KV_SEPARATOR);
				settings[line.substr(0, separator)] = line.substr(separator + 1);
			}
			saveFile.close();
			
			//Parse high scores from string to high score objects
			string& hsString = settings["highScores"];
			size_t start = 0;
			size_t end = hsString.find(character::DATA_END);
			
			while(end != string::npos) {
				highScores.insert(HighScore(hsString.substr(start, end - start)));
				start = end + 1;
				end = hsString.find(character::DATA_END, start);
			}
		}
		
		/**
		 * Save this settings object to config file
		 */
		void save() {
			settings["highScores"] = "";
			for(HighScore highScore : highScores) {
				settings["highScores"] += highScore.serialize();
			}
			
			ofstream saveFile("ms.dat");
			for(const auto& entry : settings) {
				saveFile << entry.first << character::KV_SEPARATOR << entry.second << endl;
			}
			saveFile.close();
		}
		
		const function<bool(const Minefield& field, const short& row, const short& col)>* getEvaluator() {
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
		
		const short getPixelDisplayThreshold() {
			return stoi(settings["pixelDisplayThreshold"]);
		}
		
		void setPixelDisplayThreshold(const short& threshold) {
			settings["pixelDisplayThreshold"] = to_string(threshold);
		}
		
		const string getPlayerName() {
			return settings["playerName"];
		}
		
		void setPlayerName(const string& name) {
			settings["playerName"] = name;
		}
		
		const int getSolverModeDelay() {
			return stoi(settings["solverModeDelay"]);
		}
		
		void setSolverModeDelay(const int& delayMillis) {
			settings["solverModeDelay"] = to_string(delayMillis);
		}
		
		const set<HighScore, HighScoreComparator>& getHighScores() const {
			return highScores;
		}
		
		const pair<set<HighScore, HighScoreComparator>::iterator, bool> addHighScore(const Minefield& game, const string& desiredPlayerName) {
			HighScore newHighScore = HighScore(game, desiredPlayerName);
			auto insertAttemptResult = highScores.insert(newHighScore);
			if(insertAttemptResult.second) { //No equivalent entry exists
				return insertAttemptResult;
			}
			else { //An entry already exists
				if(newHighScore.elapsedTime < insertAttemptResult.first->elapsedTime) { //That entry contains a worse time
					highScores.erase(insertAttemptResult.first);
					return highScores.insert(newHighScore);
				}
				else { //That entry contains a better time
					return insertAttemptResult;
				}
			}
		}
		
		void addHighScore(const set<HighScore, HighScoreComparator>::iterator& hint, const Minefield& game, const string& desiredPlayerName) {
			highScores.insert(hint, HighScore(game, desiredPlayerName));
		}
		
		const set<HighScore, HighScoreComparator>::iterator removeHighScore(const set<HighScore, HighScoreComparator>::iterator& highScoreToRemove) {
			return highScores.erase(highScoreToRemove);
		}
};