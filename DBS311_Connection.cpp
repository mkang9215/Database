#include <iostream>
#include <cstring>
#include <occi.h>

using oracle::occi::Environment;
using oracle::occi::Connection;
using namespace oracle::occi;
using namespace std;

struct Cart {
	int product_id;
	double price;
	int quantity;
};

int mainMenu();
int customerLogin(Connection* conn, int customerId);
int addToCart(Connection* conn, struct Cart cart[]);
double findProduct(Connection* conn, int product_id);
void displayProducts(struct Cart cart[], int productCount);
int checkout(Connection* conn, struct Cart cart[], int customerId, int productCount);
int inputRange(int min, int max);
int inputMainRange(int min, int max);
int inputInteger();

// main function
int main() {

	Environment* env = nullptr;
	Connection* conn = nullptr;

	string user = "";
	string pass = "";
	string constr = "myoracle12c.senecacollege.ca:1521/oracle12c";
	env = Environment::createEnvironment(Environment::DEFAULT);
	conn = env->createConnection(user, pass, constr);

	int menu = 1;

	try {
		while (menu) {
			menu = mainMenu();
			if (menu) {
				int customerId;
				int found;
				std::cout << "Enter the customer ID: ";
				customerId = inputRange(1, 999999);
				found = customerLogin(conn, customerId);
				if (found) {
					Cart cart[5];
					int numberItem = addToCart(conn, cart);
					if (numberItem) {
						int result = checkout(conn, cart, customerId, numberItem);
					}
				}
				else {
					std::cout << "The customer does not exist.\n";
				}
			}
		}
		std::cout << "Good bye!...\n\n";
	}
	catch (SQLException& sqlExcp) {
		std::cout << sqlExcp.getErrorCode() << ": " << sqlExcp.getMessage();
	}
	env->terminateConnection(conn);
	Environment::terminateEnvironment(env);
}

// mainMenu: to display a menu, prompt users to input a number, 
//           and return the number
int mainMenu() {
	int option = -1;
	while (option < 0) {
		std::cout << "******************** Main Menu ********************\n";
		std::cout << "1)	  Login\n";
		std::cout << "0)	  Exit\n";
		std::cout << "Enter an option (0-1): ";
		option = inputMainRange(0, 1);
	}
	return option;
}

// customerLogin: to receive a OCCI pointer and customer ID,
//                and make sure if the customer Id is in the database by calling 
//                the stored procedure
int customerLogin(Connection* conn, int customerId) {
	Statement* stmt = conn->createStatement();
	int found;
	stmt->setSQL("BEGIN find_customer(:1, :2); END;");
	stmt->setInt(1, customerId);
	stmt->registerOutParam(2, Type::OCCIINT, sizeof(found));
	stmt->executeUpdate();
	found = stmt->getInt(2);
	conn->terminateStatement(stmt);
	return found;
}

// addToCart: to receive a OCCI pointer and the array of ShoppingCart,
//            add the product users have searched for to the array,
//            and return the number of products added
int addToCart(Connection* conn, struct Cart cart[]) {
	int count = 0;
	bool flag = true;
	int check = 1;
	std::cout << "-------------- Add Products to Cart --------------\n";
	while (flag && count < 5) {
		int m_id = 0;
		double m_price = 0;
		int m_quantity = 0;
		std::cout << "Enter the product ID: ";
		m_id = inputRange(0, 999999);
		m_price = findProduct(conn, m_id);
		if (m_price) {
			std::cout << "Product Price: " << m_price << std::endl;
			std::cout << "Enter the product Quantity: ";
			m_quantity = inputRange(1, 999999);
			cart[count].product_id = m_id;
			cart[count].price = m_price;
			cart[count++].quantity = m_quantity;
			std::cout << "Enter 1 to add more products or 0 to checkout: ";
			check = inputRange(0, 1);
			if (check) {
				if (count == 5) {
					std::cout << "Your cart has a max number of items!\n";
					flag = false;
				}
			}
			else {
				flag = false;
			}
		}
		else {
			std::cout << "The product does not exists. Try again...\n";
		}
	}
	displayProducts(cart, count);
	return count;
}

// findProduct: to receive a OCCI pointer and product ID, 
//              search for the price of the product by calling the store procedure
//              and return the value of the price
double findProduct(Connection* conn, int product_id) {
	Statement* stmt = conn->createStatement();
	double m_price = 0;
	stmt->setSQL("BEGIN find_product(:1, :2); END;");
	stmt->setInt(1, product_id);
	stmt->registerOutParam(2, Type::OCCIDOUBLE, sizeof(m_price));
	stmt->executeUpdate();
	m_price = stmt->getDouble(2);
	conn->terminateStatement(stmt);
	return m_price;
}

// displayProducts: to receive the array of ShoppingCart and the number of products
//                  and display all elements stored in the array
void displayProducts(struct Cart cart[], int productCount) {
	double m_total = 0;
	std::cout << "------- Ordered Products ---------\n";
	for (size_t i = 0; i < productCount; i++) {
		std::cout << "---Item " << i + 1 << std::endl;
		std::cout << "Product ID: " << cart[i].product_id << std::endl;
		std::cout << "Price: " << cart[i].price << std::endl;
		std::cout << "Quantity: " << cart[i].quantity << std::endl;
		m_total += cart[i].quantity * cart[i].price;
	}
	std::cout << "----------------------------------\n";
	std::cout << "Total: " << m_total << std::endl;
}

// checkout: to receive a OCCI pointer, the array of ShoppingCart, customer ID, and the number of
//           products and then if a customer want, store an order and order detail in 
//           ShoppingCart by calling the stored procedures
int checkout(Connection* conn, struct Cart cart[], int customerId, int productCount) {
	int flag = 1;
	while (flag) {
		char input;
		std::cout << "Would you like to checkout? (Y/y or N/n) ";
		std::cin >> input;
		std::cin.clear();
		std::cin.ignore(2000, '\n');
		if (input == 'Y' || input == 'y') {
			Statement* stmt = conn->createStatement();
			int m_orderId;
			stmt->setSQL("BEGIN add_order(:1, :2); END;");
			stmt->setInt(1, customerId);
			stmt->registerOutParam(2, Type::OCCIINT, sizeof(m_orderId));
			stmt->executeUpdate();
			m_orderId = stmt->getInt(2);
			for (size_t i = 0; i < productCount; i++) {
				stmt->setSQL("BEGIN add_orderline(:1, :2, :3, :4, :5); END;");
				stmt->setInt(1, m_orderId);
				stmt->setInt(2, i + 1);
				stmt->setInt(3, cart[i].product_id);
				stmt->setInt(4, cart[i].quantity);
				stmt->setInt(5, cart[i].price);
				stmt->executeUpdate();
			}
			std::cout << "The order is successfully completed.\n";
			// conn->commit();
			conn->terminateStatement(stmt);
			flag = 0;
		}
		else if (input == 'N' || input == 'n') {
			std::cout << "The order is cancelled.\n";
			flag = 0;
		}
		else {
			std::cout << "Wrong input. Try again...\n";
		}
	}
	return flag;
}

// inputRange: to prompt users input a number, check if the number input 
//             is within the range and if it is, return its value
int inputRange(int min, int max) {
	bool flag = true;
	int input;
	while (flag) {
		input = inputInteger();
		std::cin.clear();
		std::cin.ignore(2000, '\n');
		if (input < min || input > max) {
			std::cout << "You entered a wrong value. Enter an option ("
				<< min << "-" << max << "): ";
		}
		else {
			flag = false;
		}
	}
	return input;
}

// inputMainRange: to prompt users input a number, check if the number input 
//                 is within the range and if it is, return its value
//                 especially, it is dedicated for mainMenu function
int inputMainRange(int min, int max) {
	bool flag = true;
	int input;
	while (flag) {
		input = inputInteger();
		std::cin.clear();
		std::cin.ignore(2000, '\n');
		if (input < min || input > max) {
			std::cout << "******************** Main Menu ********************\n";
			std::cout << "1)	  Login\n";
			std::cout << "0)	  Exit\n";
			std::cout << "You entered a wrong value. Enter an option ("
				<< min << "-" << max << "): ";
		}
		else {
			flag = false;
		}
	}
	return input;
}

// inputInteger: to prompt users input a number, 
//               check whether it is a number, and if it is, return its value
int inputInteger() {
	int input;
	while (!(std::cin >> input)) {
		std::cout << "Invalid Integer, try again: ";
		std::cin.clear();
		std::cin.ignore(2000, '\n');
	}
	return input;
}
