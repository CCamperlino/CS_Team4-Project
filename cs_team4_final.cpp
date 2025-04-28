#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <map>

using namespace std;

// ====== TERMINAL COLORS ======
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"

// ====== STRUCTS ======
struct Item {
    string id;
    string name;
    int quantity;
    int reorderPoint;
    string locationId;
};

struct Location {
    string id;
    string description;
};

struct DamagedItem {
    string itemId;
    int quantity;
};

struct Order {
    int customersownId;
    int orderId;
    string itemName;
    int amount;
    string status;
};

struct itemBin {
    string name;
    int count;
};

struct ShippingLog {
    string itemName;
    int quantity;
    string type; // "Shipped" or "Received"
    string timestamp;
};

// ====== GLOBAL VECTORS ======
vector<Item> inventory;
vector<Location> locations;
vector<DamagedItem> damagedItems;
vector<ShippingLog> shippingLogs;
itemBin bins[10];

// ====== UTILITY ======
string getLineInput(const string& prompt) {
    string input;
    cout << prompt;
    getline(cin, input);
    return input;
}

int generateIDs(int min, int max) {
    return rand() % (max - min + 1) + min;
}

// ====== ORDER FUNCTIONS ======
Order makeOrder(const string& itemName, int quantity, const string& status) {
    Order order;
    order.customersownId = generateIDs(100, 999);
    order.orderId = generateIDs(1000, 9999);
    order.itemName = itemName;
    order.amount = quantity;
    order.status = status;
    return order;
}

void detailedOrder(const Order& order) {
    cout << "\n" << GREEN << "Your Order:" << RESET << endl;
    cout << "Customer #: " << order.customersownId << "\nOrder #: " << order.orderId;
    cout << "\nProduct: " << order.itemName << "\nQuantity: " << order.amount;
    cout << "\nStatus: " << order.status << endl;
}

// ====== STOCK TRACKING ======
void syncBinsWithInventory() {
    int i = 0;
    for (const auto& item : inventory) {
        if (i < 5) {
            bins[i].name = item.name;
            bins[i].count = item.quantity;
            ++i;
        }
    }
}

// ====== LOCATION MANAGEMENT ======
void addLocation() {
    cin.ignore();
    Location loc;
    loc.id = getLineInput("Enter Location ID: ");
    loc.description = getLineInput("Enter Description: ");
    locations.push_back(loc);
    cout << GREEN << "Location added.\n" << RESET;
}

void listLocations() {
    cout << CYAN << "\n--- Defined Locations ---\n" << RESET;
    if (locations.empty()) {
        cout << RED << "No locations available.\n" << RESET;
        return;
    }
    for (const auto& loc : locations) {
        cout << "ID: " << loc.id << " | Description: " << loc.description << endl;
    }
}

// ====== INVENTORY MANAGEMENT ======
void addItem() {
    if (locations.empty()) {
        cout << RED << "No locations defined. Please add a location first.\n" << RESET;
        return;
    }

    cin.ignore();
    Item newItem;
    newItem.id = getLineInput("Enter Item ID: ");
    newItem.name = getLineInput("Enter Item Name: ");

    cout << "Enter Quantity: ";
    while (!(cin >> newItem.quantity) || newItem.quantity < 0) {
        cout << RED << "Invalid quantity. Enter again: " << RESET;
        cin.clear(); cin.ignore(1000, '\n');
    }

    cout << "Enter Reorder Point: ";
    while (!(cin >> newItem.reorderPoint) || newItem.reorderPoint < 0) {
        cout << RED << "Invalid reorder point. Enter again: " << RESET;
        cin.clear(); cin.ignore(1000, '\n');
    }

    listLocations();
    cin.ignore();

    bool validLocation = false;
    do {
        newItem.locationId = getLineInput("Enter Location ID from list: ");
        for (const auto& loc : locations) {
            if (loc.id == newItem.locationId) {
                validLocation = true;
                break;
            }
        }
        if (!validLocation) cout << RED << "Invalid Location ID. Try again.\n" << RESET;
    } while (!validLocation);

    inventory.push_back(newItem);
    syncBinsWithInventory();
    cout << GREEN << "Item added successfully.\n" << RESET;
}

void listItems() {
    cout << CYAN << "\n--- Inventory Items ---\n" << RESET;
    if (inventory.empty()) {
        cout << RED << "No items found.\n" << RESET;
        return;
    }
    for (const auto& item : inventory) {
        cout << "ID: " << item.id << " | " << item.name << " | Qty: " << item.quantity;
        cout << " | Reorder: " << item.reorderPoint << " | Loc: " << item.locationId;
        if (item.quantity <= item.reorderPoint) cout << YELLOW << " (LOW STOCK!)" << RESET;
        cout << endl;
    }
}

// ====== REORDER POINT CHECK ======
void checkReorderAlerts() {
    cout << CYAN << "\n--- Reorder Point Alerts ---\n" << RESET;
    bool found = false;
    for (const auto& item : inventory) {
        if (item.quantity <= item.reorderPoint) {
            cout << YELLOW << "ALERT: " << item.name << " (Qty: " << item.quantity << ")" << RESET << endl;
            found = true;
        }
    }
    if (!found) cout << GREEN << "All stocks above reorder levels.\n" << RESET;
}

// ====== SEARCH ITEMS ======
void searchInventory() {
    cout << CYAN << "\nSearch Inventory (case sensitive, 'exit' to stop)\n" << RESET;
    string input;
    cin.ignore();
    while (true) {
        cout << "Item name: ";
        getline(cin, input);
        if (input == "exit") break;
        bool found = false;
        for (const auto& item : inventory) {
            if (item.name.find(input) != string::npos) {
                cout << GREEN << "Found: " << item.name << " (Qty: " << item.quantity << ")\n" << RESET;
                found = true;
            }
        }
        if (!found) cout << RED << "Item not found.\n" << RESET;
    }
}

// ====== ORDER FULFILLMENT ======
void placeOrder() {
    cin.ignore();
    string name;
    int qty;
    cout << "Enter product: ";
    getline(cin, name);
    cout << "Enter quantity: ";
    cin >> qty;

    if (qty <= 0) {
        cout << RED << "Invalid quantity.\n" << RESET;
        return;
    }

    for (auto& item : inventory) {
        if (item.name == name) {
            if (item.quantity < qty) {
                cout << RED << "Not enough stock!\n" << RESET;
                return;
            }
            item.quantity -= qty;
            syncBinsWithInventory();

            Order newOrder = makeOrder(name, qty, "Fulfilled");
            detailedOrder(newOrder);

            ShippingLog log = {name, qty, "Shipped", "Today"};
            shippingLogs.push_back(log);
            return;
        }
    }
    cout << RED << "Product not found.\n" << RESET;
}

// ====== DAMAGE FUNCTIONS ======
void reportDamagedItem() {
    cin.ignore();
    string name = getLineInput("Enter item name: ");
    int qty;
    cout << "Enter quantity damaged: ";
    cin >> qty;

    for (auto& item : inventory) {
        if (item.name == name) {
            if (item.quantity >= qty) {
                item.quantity -= qty;
                damagedItems.push_back({item.id, qty});
                syncBinsWithInventory();
                cout << YELLOW << "Damage reported.\n" << RESET;
            } else cout << RED << "Not enough stock.\n" << RESET;
            return;
        }
    }
    cout << RED << "Item not found.\n" << RESET;
}

void showDamagedItems() {
    cout << CYAN << "\nDamaged Items Log\n" << RESET;
    if (damagedItems.empty()) {
        cout << GREEN << "No damage reports.\n" << RESET;
        return;
    }
    for (const auto& d : damagedItems) {
        cout << "Item ID: " << d.itemId << " | Quantity: " << d.quantity << endl;
    }
}

// ====== BINS ======
void displayBins() {
    cout << CYAN << "\n--- Bins ---\n" << RESET;
    for (int i = 0; i < 5; ++i) {
        cout << "Bin#" << i+1 << ": " << bins[i].name << " (" << bins[i].count << " items)\n";
    }
}

// ====== INVENTORY REPORT ======
void generateInventoryReport() {
    cout << CYAN << "\n--- Inventory Report ---\n" << RESET;
    cout << "Items: " << inventory.size() << "\nDamaged: " << damagedItems.size() << "\n";
}

// ====== MAIN MENU ======
int main() {
    srand(time(0));
    syncBinsWithInventory();

    int choice;
    do {
        cout << CYAN << "\n==== Inventory Management ====\n" << RESET;
        cout << "1. Add Item\n2. List Items\n3. Add Location\n4. List Locations\n5. Report/View Damage\n";
        cout << "6. Reorder Alerts\n7. Save/Load Data\n8. Search Inventory\n9. Place Order\n";
        cout << "10. View Bins\n11. Generate Report\n0. Exit\nChoice: ";
        cin >> choice;

        switch (choice) {
            case 1: addItem(); break;
            case 2: listItems(); break;
            case 3: addLocation(); break;
            case 4: listLocations(); break;
            case 5: { int x; cout << "1. Report 2. View: "; cin >> x; if (x==1) reportDamagedItem(); else showDamagedItems(); } break;
            case 6: checkReorderAlerts(); break;
            case 7: cout << YELLOW << "(Data persistence feature placeholder.)\n" << RESET; break;
            case 8: searchInventory(); break;
            case 9: placeOrder(); break;
            case 10: displayBins(); break;
            case 11: generateInventoryReport(); break;
            case 0: cout << "Goodbye.\n"; break;
            default: cout << RED << "Invalid option!\n" << RESET; break;
        }
    } while (choice != 0);

    return 0;
}
