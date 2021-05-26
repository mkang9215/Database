SET SERVEROUTPUT ON;
ROLLBACK;

-- find_customer 
CREATE OR REPLACE PROCEDURE find_customer(customer_id IN NUMBER, found OUT NUMBER) AS
BEGIN
    SELECT COUNT(*)
    INTO found
    FROM customers
    WHERE cust_no = customer_id;    
EXCEPTION
    WHEN NO_DATA_FOUND THEN
        found := 0;
        DBMS_OUTPUT.PUT_LINE ('No data found!');
    WHEN OTHERS THEN
        found := 0;
        DBMS_OUTPUT.PUT_LINE('Error!');
END find_customer;


-- find_product
CREATE OR REPLACE PROCEDURE find_product(product_id IN NUMBER, price OUT products.prod_sell%TYPE) AS
BEGIN
    SELECT DISTINCT prod_sell
    INTO    price
    FROM    products
    WHERE   prod_no = product_id;    
EXCEPTION
    WHEN NO_DATA_FOUND THEN
        price := 0;
        DBMS_OUTPUT.PUT_LINE ('No data found!');
    WHEN OTHERS THEN
        DBMS_OUTPUT.PUT_LINE ('Error!');
        price := 0;
END find_product;


-- add_order
CREATE OR REPLACE PROCEDURE add_order(customer_id IN NUMBER, new_order_id OUT NUMBER) AS
BEGIN
    SELECT  MAX(order_no)
    INTO    new_order_id
    FROM    orders;
    
    new_order_id := new_order_id + 1;
    
    INSERT INTO orders(order_no, rep_no, cust_no, order_dt, status)
    VALUES(new_order_id, 36, customer_id, Sysdate, 'C');
EXCEPTION
    WHEN OTHERS THEN
    DBMS_OUTPUT.PUT_LINE ('Error!');
END add_order;

-- add_orderline
CREATE OR REPLACE PROCEDURE add_orderline (orderId IN orderlines.order_no%type,
                                        itemId IN orderlines.line_no%type, 
                                        productId IN orderlines.prod_no%type, 
                                        quantity IN orderlines.qty%type,
                                        price IN orderlines.price%type) AS
BEGIN
    INSERT INTO orderlines(order_no, line_no, prod_no, price, qty)
    VALUES(orderId, itemId, productId, price, quantity);
EXCEPTION
    WHEN OTHERS THEN
    DBMS_OUTPUT.PUT_LINE ('Error!');    
END add_orderline;


--SELECT
SELECT cname, o.order_no, prod_name, price*qty
FROM customers c JOIN orders o ON (c.cust_no = o.cust_no)
JOIN orderlines l ON (o.order_no = l.order_no)
JOIN products p ON (l.prod_no = p.prod_no)
WHERE order_dt LIKE '21/%';
