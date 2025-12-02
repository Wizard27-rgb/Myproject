#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <random>
#include <iomanip>
#include <cmath>

#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(x) Sleep(x)
    #define SLEEP_SEC(x) Sleep(x * 1000)
#else
    #include <unistd.h>
    #define SLEEP_MS(x) usleep(x * 1000)
    #define SLEEP_SEC(x) sleep(x)
#endif

using namespace std;

// ==================== ENCRYPTION & SECURITY ====================
class SecurityManager {
private:
    string key;
    
    // Simple XOR encryption (for demo - use OpenSSL/AES in production)
    string xorEncrypt(const string& data, const string& key) {
        string result = data;
        for(size_t i = 0; i < data.length(); i++) {
            result[i] = data[i] ^ key[i % key.length()];
        }
        return toHex(result);
    }
    
    string xorDecrypt(const string& hexData, const string& key) {
        string data = fromHex(hexData);
        string result = data;
        for(size_t i = 0; i < data.length(); i++) {
            result[i] = data[i] ^ key[i % key.length()];
        }
        return result;
    }
    
    string toHex(const string& data) {
        stringstream ss;
        ss << hex << setfill('0');
        for(unsigned char c : data) {
            ss << setw(2) << (int)c;
        }
        return ss.str();
    }
    
    string fromHex(const string& hexData) {
        string result;
        for(size_t i = 0; i < hexData.length(); i += 2) {
            string byte = hexData.substr(i, 2);
            char chr = (char)strtol(byte.c_str(), nullptr, 16);
            result.push_back(chr);
        }
        return result;
    }

public:
    SecurityManager(const string& masterPassword) {
        // Create encryption key from master password
        key = hashPassword(masterPassword);
    }
    
    string hashPassword(const string& password) {
        // Simple hash (use SHA-256 in production)
        unsigned long hash = 5381;
        for(char c : password) {
            hash = ((hash << 5) + hash) + c;
        }
        return to_string(hash);
    }
    
    string encrypt(const string& data) {
        return xorEncrypt(data, key);
    }
    
    string decrypt(const string& encryptedData) {
        return xorDecrypt(encryptedData, key);
    }
};

// ==================== PASSWORD ANALYZER ====================
class PasswordAnalyzer {
public:
    struct PasswordStrength {
        int score;          // 0-100
        string strength;    // Weak, Fair, Good, Strong, Very Strong
        vector<string> feedback;
        double entropy;
    };
    
    static PasswordStrength analyzePassword(const string& password) {
        PasswordStrength result;
        result.score = 0;
        
        int length = password.length();
        bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        
        // Check character types
        for(char c : password) {
            if(isupper(c)) hasUpper = true;
            if(islower(c)) hasLower = true;
            if(isdigit(c)) hasDigit = true;
            if(ispunct(c)) hasSpecial = true;
        }
        
        // Calculate score
        if(length >= 8) result.score += 20;
        if(length >= 12) result.score += 15;
        if(length >= 16) result.score += 15;
        if(hasUpper) result.score += 15;
        if(hasLower) result.score += 15;
        if(hasDigit) result.score += 10;
        if(hasSpecial) result.score += 10;
        
        // Calculate entropy
        int charsetSize = 0;
        if(hasUpper) charsetSize += 26;
        if(hasLower) charsetSize += 26;
        if(hasDigit) charsetSize += 10;
        if(hasSpecial) charsetSize += 32;
        result.entropy = length * log2(charsetSize);
        
        // Determine strength
        if(result.score < 40) result.strength = "Weak";
        else if(result.score < 60) result.strength = "Fair";
        else if(result.score < 75) result.strength = "Good";
        else if(result.score < 90) result.strength = "Strong";
        else result.strength = "Very Strong";
        
        // Generate feedback
        if(length < 8) result.feedback.push_back("• Use at least 8 characters");
        if(length < 12) result.feedback.push_back("• Consider using 12+ characters");
        if(!hasUpper) result.feedback.push_back("• Add uppercase letters");
        if(!hasLower) result.feedback.push_back("• Add lowercase letters");
        if(!hasDigit) result.feedback.push_back("• Add numbers");
        if(!hasSpecial) result.feedback.push_back("• Add special characters (!@#$%)");
        
        if(result.feedback.empty()) {
            result.feedback.push_back("✓ Excellent password!");
        }
        
        return result;
    }
};

// ==================== PASSWORD GENERATOR ====================
class PasswordGenerator {
public:
    static string generate(int length = 16, bool useUpper = true, 
                          bool useLower = true, bool useDigits = true, 
                          bool useSpecial = true) {
        string charset = "";
        if(useUpper) charset += "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        if(useLower) charset += "abcdefghijklmnopqrstuvwxyz";
        if(useDigits) charset += "0123456789";
        if(useSpecial) charset += "!@#$%^&*()_+-=[]{}|;:,.<>?";
        
        if(charset.empty()) charset = "abcdefghijklmnopqrstuvwxyz";
        
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, charset.length() - 1);
        
        string password;
        for(int i = 0; i < length; i++) {
            password += charset[dis(gen)];
        }
        
        return password;
    }
};

// ==================== PASSWORD ENTRY ====================
struct PasswordEntry {
    string id;
    string website;
    string username;
    string password;
    string category;
    string notes;
    time_t createdAt;
    time_t lastModified;
    
    PasswordEntry() : createdAt(time(0)), lastModified(time(0)) {}
};

// ==================== PASSVAULT MANAGER ====================
class PassVault {
private:
    string vaultFile;
    SecurityManager* security;
    vector<PasswordEntry> entries;
    string masterPassword;
    bool isLocked;
    time_t lastActivity;
    int autoLockMinutes;
    
    string generateId() {
        return to_string(time(0)) + to_string(rand() % 10000);
    }
    
    void updateActivity() {
        lastActivity = time(0);
    }
    
    bool checkAutoLock() {
        if(autoLockMinutes > 0) {
            time_t now = time(0);
            if(difftime(now, lastActivity) > autoLockMinutes * 60) {
                isLocked = true;
                return true;
            }
        }
        return false;
    }

public:
    PassVault(const string& filename) : vaultFile(filename), security(nullptr), 
                                         isLocked(true), autoLockMinutes(10) {
        lastActivity = time(0);
    }
    
    ~PassVault() {
        if(security) delete security;
    }
    
    bool initialize(const string& masterPass) {
        masterPassword = masterPass;
        security = new SecurityManager(masterPassword);
        isLocked = false;
        updateActivity();
        return true;
    }
    
    void lock() {
        isLocked = true;
    }
    
    bool unlock(const string& masterPass) {
        if(masterPass == masterPassword) {
            isLocked = false;
            updateActivity();
            return true;
        }
        return false;
    }
    
    bool addEntry(const PasswordEntry& entry) {
        if(checkAutoLock() || isLocked) return false;
        updateActivity();
        
        PasswordEntry newEntry = entry;
        newEntry.id = generateId();
        newEntry.createdAt = time(0);
        newEntry.lastModified = time(0);
        entries.push_back(newEntry);
        return saveToFile();
    }
    
    bool updateEntry(const string& id, const PasswordEntry& entry) {
        if(checkAutoLock() || isLocked) return false;
        updateActivity();
        
        for(auto& e : entries) {
            if(e.id == id) {
                e.website = entry.website;
                e.username = entry.username;
                e.password = entry.password;
                e.category = entry.category;
                e.notes = entry.notes;
                e.lastModified = time(0);
                return saveToFile();
            }
        }
        return false;
    }
    
    bool deleteEntry(const string& id) {
        if(checkAutoLock() || isLocked) return false;
        updateActivity();
        
        auto it = remove_if(entries.begin(), entries.end(),
                           [&id](const PasswordEntry& e) { return e.id == id; });
        if(it != entries.end()) {
            entries.erase(it, entries.end());
            return saveToFile();
        }
        return false;
    }
    
    vector<PasswordEntry> searchEntries(const string& query) {
        if(checkAutoLock() || isLocked) return {};
        updateActivity();
        
        vector<PasswordEntry> results;
        string lowerQuery = query;
        transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
        
        for(const auto& entry : entries) {
            string lowerWebsite = entry.website;
            string lowerUsername = entry.username;
            transform(lowerWebsite.begin(), lowerWebsite.end(), lowerWebsite.begin(), ::tolower);
            transform(lowerUsername.begin(), lowerUsername.end(), lowerUsername.begin(), ::tolower);
            
            if(lowerWebsite.find(lowerQuery) != string::npos ||
               lowerUsername.find(lowerQuery) != string::npos ||
               entry.category.find(lowerQuery) != string::npos) {
                results.push_back(entry);
            }
        }
        return results;
    }
    
    vector<PasswordEntry> getAllEntries() {
        if(checkAutoLock() || isLocked) return {};
        updateActivity();
        return entries;
    }
    
    PasswordEntry* getEntry(const string& id) {
        if(checkAutoLock() || isLocked) return nullptr;
        updateActivity();
        
        for(auto& entry : entries) {
            if(entry.id == id) return &entry;
        }
        return nullptr;
    }
    
    bool saveToFile() {
        ofstream file(vaultFile);
        if(!file.is_open()) return false;
        
        for(const auto& entry : entries) {
            file << security->encrypt(entry.id) << "|"
                 << security->encrypt(entry.website) << "|"
                 << security->encrypt(entry.username) << "|"
                 << security->encrypt(entry.password) << "|"
                 << security->encrypt(entry.category) << "|"
                 << security->encrypt(entry.notes) << "|"
                 << entry.createdAt << "|"
                 << entry.lastModified << "\n";
        }
        file.close();
        return true;
    }
    
    bool loadFromFile() {
        ifstream file(vaultFile);
        if(!file.is_open()) return false;
        
        entries.clear();
        string line;
        while(getline(file, line)) {
            stringstream ss(line);
            PasswordEntry entry;
            string temp;
            
            getline(ss, temp, '|'); entry.id = security->decrypt(temp);
            getline(ss, temp, '|'); entry.website = security->decrypt(temp);
            getline(ss, temp, '|'); entry.username = security->decrypt(temp);
            getline(ss, temp, '|'); entry.password = security->decrypt(temp);
            getline(ss, temp, '|'); entry.category = security->decrypt(temp);
            getline(ss, temp, '|'); entry.notes = security->decrypt(temp);
            getline(ss, temp, '|'); entry.createdAt = stol(temp);
            getline(ss, temp, '|'); entry.lastModified = stol(temp);
            
            entries.push_back(entry);
        }
        file.close();
        return true;
    }
    
    map<string, int> getHealthReport() {
        map<string, int> report;
        report["total"] = entries.size();
        report["weak"] = 0;
        report["reused"] = 0;
        report["old"] = 0;
        
        map<string, int> passwordCount;
        time_t now = time(0);
        
        for(const auto& entry : entries) {
            // Check weak passwords
            auto strength = PasswordAnalyzer::analyzePassword(entry.password);
            if(strength.score < 60) report["weak"]++;
            
            // Check reused passwords
            passwordCount[entry.password]++;
            
            // Check old passwords (older than 6 months)
            double monthsOld = difftime(now, entry.lastModified) / (30 * 24 * 60 * 60);
            if(monthsOld > 6) report["old"]++;
        }
        
        for(const auto& p : passwordCount) {
            if(p.second > 1) report["reused"] += p.second;
        }
        
        return report;
    }
};

// ==================== UI HELPER ====================
class UIHelper {
public:
    static void clearScreen() {
        #ifdef _WIN32
            system("cls");
        #else
            system("clear");
        #endif
    }
    
    static void printHeader(const string& title) {
        cout << "\n╔════════════════════════════════════════════════════════════╗\n";
        cout << "║" << setw(35 + title.length()/2) << title 
             << setw(35 - title.length()/2) << "║\n";
        cout << "╚════════════════════════════════════════════════════════════╝\n\n";
    }
    
    static void printBox(const string& content) {
        cout << "┌────────────────────────────────────────────────────────────┐\n";
        cout << "│ " << content << setw(59 - content.length()) << "│\n";
        cout << "└────────────────────────────────────────────────────────────┘\n";
    }
    
    static void printProgressBar(int percentage) {
        cout << "[";
        int bars = percentage / 5;
        for(int i = 0; i < 20; i++) {
            if(i < bars) cout << "█";
            else cout << "░";
        }
        cout << "] " << percentage << "%";
    }
    
    static void showLoading(const string& message) {
        cout << message;
        for(int i = 0; i < 3; i++) {
            cout << "." << flush;
            SLEEP_MS(300);
        }
        cout << "\n";
    }
};

// ==================== MAIN APPLICATION ====================
int main() {
    PassVault vault("passvault.dat");
    string masterPassword;
    bool running = true;
    
    UIHelper::clearScreen();
    UIHelper::printHeader("🔐 PASSVAULT - Advanced Password Manager");
    
    cout << "Welcome to PassVault!\n\n";
    cout << "Please set your master password: ";
    getline(cin, masterPassword);
    
    if(masterPassword.length() < 6) {
        cout << "\n⚠ Master password must be at least 6 characters!\n";
        return 1;
    }
    
    vault.initialize(masterPassword);
    vault.loadFromFile();
    
    UIHelper::showLoading("Initializing secure vault");
    cout << "✓ Vault unlocked successfully!\n";
    SLEEP_SEC(1);
    
    while(running) {
        UIHelper::clearScreen();
        UIHelper::printHeader("🔐 PASSVAULT MENU");
        
        // Show health report
        auto health = vault.getHealthReport();
        cout << "📊 Vault Health: " << health["total"] << " passwords stored";
        if(health["weak"] > 0) cout << " | ⚠ " << health["weak"] << " weak";
        if(health["reused"] > 0) cout << " | ⚠ " << health["reused"] << " reused";
        if(health["old"] > 0) cout << " | ⚠ " << health["old"] << " old";
        cout << "\n\n";
        
        cout << "1. Add New Password\n";
        cout << "2. View All Passwords\n";
        cout << "3. Search Passwords\n";
        cout << "4. Generate Strong Password\n";
        cout << "5. Password Health Dashboard\n";
        cout << "6. Update Password\n";
        cout << "7. Delete Password\n";
        cout << "8. Lock Vault\n";
        cout << "9. Exit\n\n";
        cout << "Choose an option: ";
        
        int choice;
        cin >> choice;
        cin.ignore();
        
        switch(choice) {
            case 1: {
                UIHelper::clearScreen();
                UIHelper::printHeader("➕ ADD NEW PASSWORD");
                
                PasswordEntry entry;
                cout << "Website/Service: ";
                getline(cin, entry.website);
                cout << "Username/Email: ";
                getline(cin, entry.username);
                
                cout << "\n1. Enter password manually\n2. Generate strong password\nChoose: ";
                int passChoice;
                cin >> passChoice;
                cin.ignore();
                
                if(passChoice == 2) {
                    entry.password = PasswordGenerator::generate(16, true, true, true, true);
                    cout << "\n✓ Generated password: " << entry.password << "\n";
                } else {
                    cout << "Password: ";
                    getline(cin, entry.password);
                }
                
                // Analyze password
                auto strength = PasswordAnalyzer::analyzePassword(entry.password);
                cout << "\n📊 Password Strength: " << strength.strength << " ";
                UIHelper::printProgressBar(strength.score);
                cout << "\n   Entropy: " << fixed << setprecision(1) << strength.entropy << " bits\n";
                for(const auto& fb : strength.feedback) {
                    cout << "   " << fb << "\n";
                }
                
                cout << "\nCategory (Banking/Social/Email/Work/Other): ";
                getline(cin, entry.category);
                cout << "Notes (optional): ";
                getline(cin, entry.notes);
                
                if(vault.addEntry(entry)) {
                    cout << "\n✓ Password saved successfully!\n";
                } else {
                    cout << "\n✗ Failed to save password!\n";
                }
                
                cout << "\nPress Enter to continue...";
                cin.get();
                break;
            }
            
            case 2: {
                UIHelper::clearScreen();
                UIHelper::printHeader("📋 ALL PASSWORDS");
                
                auto entries = vault.getAllEntries();
                if(entries.empty()) {
                    cout << "No passwords stored yet.\n";
                } else {
                    for(size_t i = 0; i < entries.size(); i++) {
                        cout << "\n" << (i+1) << ". " << entries[i].website << "\n";
                        cout << "   👤 " << entries[i].username << "\n";
                        cout << "   🔑 " << string(entries[i].password.length(), '*') << "\n";
                        cout << "   📁 " << entries[i].category << "\n";
                        if(!entries[i].notes.empty()) {
                            cout << "   📝 " << entries[i].notes << "\n";
                        }
                    }
                }
                
                cout << "\nPress Enter to continue...";
                cin.get();
                break;
            }
            
            case 3: {
                UIHelper::clearScreen();
                UIHelper::printHeader("🔍 SEARCH PASSWORDS");
                
                string query;
                cout << "Enter search term: ";
                getline(cin, query);
                
                auto results = vault.searchEntries(query);
                if(results.empty()) {
                    cout << "\nNo matches found.\n";
                } else {
                    cout << "\nFound " << results.size() << " result(s):\n";
                    for(size_t i = 0; i < results.size(); i++) {
                        cout << "\n" << (i+1) << ". " << results[i].website << "\n";
                        cout << "   👤 " << results[i].username << "\n";
                        cout << "   🔑 Password: " << results[i].password << "\n";
                        cout << "   📁 " << results[i].category << "\n";
                    }
                }
                
                cout << "\nPress Enter to continue...";
                cin.get();
                break;
            }
            
            case 4: {
                UIHelper::clearScreen();
                UIHelper::printHeader("🎲 GENERATE STRONG PASSWORD");
                
                int length;
                char useUpper, useLower, useDigits, useSpecial;
                
                cout << "Password length (8-32): ";
                cin >> length;
                length = max(8, min(32, length));
                
                cout << "Include uppercase letters? (y/n): ";
                cin >> useUpper;
                cout << "Include lowercase letters? (y/n): ";
                cin >> useLower;
                cout << "Include digits? (y/n): ";
                cin >> useDigits;
                cout << "Include special characters? (y/n): ";
                cin >> useSpecial;
                
                string password = PasswordGenerator::generate(
                    length,
                    useUpper == 'y',
                    useLower == 'y',
                    useDigits == 'y',
                    useSpecial == 'y'
                );
                
                cout << "\n✓ Generated password: " << password << "\n";
                
                auto strength = PasswordAnalyzer::analyzePassword(password);
                cout << "\n📊 Password Strength: " << strength.strength << " ";
                UIHelper::printProgressBar(strength.score);
                cout << "\n   Entropy: " << fixed << setprecision(1) << strength.entropy << " bits\n";
                
                cout << "\nPress Enter to continue...";
                cin.ignore();
                cin.get();
                break;
            }
            
            case 5: {
                UIHelper::clearScreen();
                UIHelper::printHeader("📊 PASSWORD HEALTH DASHBOARD");
                
                auto health = vault.getHealthReport();
                
                cout << "Total Passwords: " << health["total"] << "\n\n";
                cout << "⚠ Weak Passwords: " << health["weak"] << "\n";
                cout << "⚠ Reused Passwords: " << health["reused"] << "\n";
                cout << "⚠ Old Passwords (6+ months): " << health["old"] << "\n\n";
                
                int healthScore = 100;
                if(health["total"] > 0) {
                    healthScore -= (health["weak"] * 30 / health["total"]);
                    healthScore -= (health["reused"] * 30 / health["total"]);
                    healthScore -= (health["old"] * 20 / health["total"]);
                }
                
                cout << "Overall Security Score: ";
                UIHelper::printProgressBar(max(0, healthScore));
                cout << "\n\n";
                
                if(healthScore < 70) {
                    cout << "💡 Recommendation: Update weak and reused passwords immediately!\n";
                } else if(healthScore < 90) {
                    cout << "💡 Recommendation: Consider updating old passwords.\n";
                } else {
                    cout << "✓ Your password security looks great!\n";
                }
                
                cout << "\nPress Enter to continue...";
                cin.get();
                break;
            }
            
            case 6: {
                UIHelper::clearScreen();
                UIHelper::printHeader("✏️ UPDATE PASSWORD");
                
                auto entries = vault.getAllEntries();
                if(entries.empty()) {
                    cout << "No passwords stored yet.\n";
                } else {
                    cout << "Select password to update:\n\n";
                    for(size_t i = 0; i < entries.size(); i++) {
                        cout << (i+1) << ". " << entries[i].website 
                             << " (" << entries[i].username << ")\n";
                    }
                    
                    cout << "\nEnter number (0 to cancel): ";
                    int num;
                    cin >> num;
                    cin.ignore();
                    
                    if(num > 0 && num <= (int)entries.size()) {
                        PasswordEntry updatedEntry = entries[num-1];
                        
                        cout << "\nUpdating: " << updatedEntry.website << "\n";
                        cout << "Leave blank to keep current value\n\n";
                        
                        cout << "New Website/Service [" << updatedEntry.website << "]: ";
                        string temp;
                        getline(cin, temp);
                        if(!temp.empty()) updatedEntry.website = temp;
                        
                        cout << "New Username/Email [" << updatedEntry.username << "]: ";
                        getline(cin, temp);
                        if(!temp.empty()) updatedEntry.username = temp;
                        
                        cout << "\n1. Enter new password manually\n2. Generate strong password\n3. Keep current\nChoose: ";
                        int passChoice;
                        cin >> passChoice;
                        cin.ignore();
                        
                        if(passChoice == 1) {
                            cout << "New Password: ";
                            getline(cin, updatedEntry.password);
                            
                            auto strength = PasswordAnalyzer::analyzePassword(updatedEntry.password);
                            cout << "\n📊 Password Strength: " << strength.strength << " ";
                            UIHelper::printProgressBar(strength.score);
                            cout << "\n";
                        } else if(passChoice == 2) {
                            updatedEntry.password = PasswordGenerator::generate(16, true, true, true, true);
                            cout << "\n✓ Generated password: " << updatedEntry.password << "\n";
                            
                            auto strength = PasswordAnalyzer::analyzePassword(updatedEntry.password);
                            cout << "\n📊 Password Strength: " << strength.strength << " ";
                            UIHelper::printProgressBar(strength.score);
                            cout << "\n";
                        }
                        
                        cout << "\nNew Category [" << updatedEntry.category << "]: ";
                        getline(cin, temp);
                        if(!temp.empty()) updatedEntry.category = temp;
                        
                        cout << "New Notes [" << updatedEntry.notes << "]: ";
                        getline(cin, temp);
                        if(!temp.empty()) updatedEntry.notes = temp;
                        
                        if(vault.updateEntry(entries[num-1].id, updatedEntry)) {
                            cout << "\n✓ Password updated successfully!\n";
                        } else {
                            cout << "\n✗ Failed to update password!\n";
                        }
                    }
                }
                
                cout << "\nPress Enter to continue...";
                cin.get();
                break;
            }
            
            case 7: {
                UIHelper::clearScreen();
                UIHelper::printHeader("🗑️ DELETE PASSWORD");
                
                auto entries = vault.getAllEntries();
                if(entries.empty()) {
                    cout << "No passwords stored yet.\n";
                } else {
                    cout << "Select password to delete:\n\n";
                    for(size_t i = 0; i < entries.size(); i++) {
                        cout << (i+1) << ". " << entries[i].website 
                             << " (" << entries[i].username << ")\n";
                    }
                    
                    cout << "\nEnter number (0 to cancel): ";
                    int num;
                    cin >> num;
                    cin.ignore();
                    
                    if(num > 0 && num <= (int)entries.size()) {
                        cout << "\n⚠️ Are you sure you want to delete '" 
                             << entries[num-1].website << "'? (y/n): ";
                        char confirm;
                        cin >> confirm;
                        cin.ignore();
                        
                        if(confirm == 'y' || confirm == 'Y') {
                            if(vault.deleteEntry(entries[num-1].id)) {
                                cout << "\n✓ Password deleted successfully!\n";
                            } else {
                                cout << "\n✗ Failed to delete password!\n";
                            }
                        } else {
                            cout << "\n✗ Deletion cancelled.\n";
                        }
                    }
                }
                
                cout << "\nPress Enter to continue...";
                cin.get();
                break;
            }
            
            case 8: {
                vault.lock();
                cout << "\n🔒 Vault locked. Goodbye!\n";
                running = false;
                break;

            }
            
            case 9: {
                vault.saveToFile();
                cout << "\n💾 Vault saved. Goodbye!\n";
                running = false;
                break;
            }
            
            default:
                cout << "\nInvalid option!\n";
                SLEEP_SEC(1);
        }
    }
    
    return 0;
}
