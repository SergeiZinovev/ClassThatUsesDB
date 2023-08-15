#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <iostream>
#include <string>

// DBConnection class interface
class IDBConnection
{
public:
    IDBConnection() {}
    virtual ~IDBConnection() {}

    virtual bool open() = 0;
    virtual bool close() = 0;
    virtual bool execQuery(const std::string& query) = 0;
};

// inherit from the interface
class DBConnection : virtual public IDBConnection
{
public:
    DBConnection() {
        std::cout << "Constructor DBConnection " << this << "\n";
        descriptor = nullptr;
    }

    ~DBConnection() {
        std::cout << "Destructor DBConnection " << this << "\n";
        delete descriptor;
    }

    bool open() override {
        if ((descriptor == nullptr) || (*descriptor < 0))
        {
            descriptor = new int(100);
            return true;
        }
        else 
            return false;
    }

    bool execQuery(const std::string& query) override {
        if (descriptor == nullptr)
            return false;
        else if (*descriptor < 0)
            return false;
        else
        {
            std::cout << "Query is " << query << "\n";
            return true;
        }
    }

    bool close() override {
        if (descriptor == nullptr) 
            return false;
        else {
            *descriptor = -1;
            return true;
        }
    }
private:
    int *descriptor; // connection descriptor
};

// Mock class
class MockDBConnection : virtual public IDBConnection
{
public:
    MOCK_METHOD(bool, open, (), (override));
    MOCK_METHOD(bool, close, (), (override));
    MOCK_METHOD(bool, execQuery, (const std::string& query), (override));
};

class ClassThatUsesDB
{
public:
    ClassThatUsesDB(IDBConnection* connection)
        : _connection(connection) {}

    bool openConnection()
    {
        if (_connection->open())
        {
            std::cout << "Connection is OK\n";
            return true;
        }
        else
        {
            std::cout << "Connection is NOT OK\n";
            return false;
        }

    }
    bool useConnection(const std::string& query)
    {
        if (_connection->execQuery(query))
        {
            std::cout << "Query is Execute\n";
            return true;
        }
        else
        {
            std::cout << "Query is Not Execute\n";
            return false;
        }
    }
    bool closeConnection()
    {
        if (_connection->close())
        {
            std::cout << "Connection is close\n";
            return true;
        }
        else
        {
            std::cout << "Connection close is failed\n";
            return false;
        }
    }

private:
    IDBConnection* _connection;
};

class SomeTestSuite : public ::testing::Test
{
protected:
    void SetUp()
    {
        _dbconn = new DBConnection();
        _dbuses = new ClassThatUsesDB(_dbconn);
    }

    void TearDown()
    {
        delete _dbuses;
        delete _dbconn;
    }

protected:
    IDBConnection* _dbconn{};
    ClassThatUsesDB* _dbuses{};
};

TEST_F(SomeTestSuite, testcase1) // database connection
{
    bool test = _dbuses->openConnection();
    ASSERT_EQ(test, true);
}

TEST_F(SomeTestSuite, testcase2) // establishing a connection and sending a query
{
    _dbuses->openConnection();
    bool test = _dbuses->useConnection("Create table");
    ASSERT_EQ(test, true);
}

TEST_F(SomeTestSuite, testcase3) // sending a request without establishing a connection
{
    bool test = _dbuses->useConnection("Create table");
    ASSERT_EQ(test, false);
}

TEST_F(SomeTestSuite, testcase4) // establishing a connection, sending a request and closing the connection
{
    _dbuses->openConnection();
    _dbuses->useConnection("Create table");
    bool test = _dbuses->closeConnection();
    ASSERT_EQ(test, true);
}

TEST_F(SomeTestSuite, testcase5) // connection closing
{
    bool test = _dbuses->closeConnection();
    ASSERT_EQ(test, false);
}

TEST_F(SomeTestSuite, testcase6) // Mock testing
{
    MockDBConnection mock;
    
    EXPECT_CALL(mock, open).WillOnce(::testing::Return(true));
    EXPECT_CALL(mock, execQuery("Create table (GMock)")).WillOnce(::testing::Return(true));
    EXPECT_CALL(mock, close).WillOnce(::testing::Return(true));

    ClassThatUsesDB dbuses(&mock);
    bool test1 = dbuses.openConnection();
    bool test2 = dbuses.useConnection("Create table (GMock)");
    bool test3 = dbuses.closeConnection();

    ASSERT_EQ(test1, true);
    ASSERT_EQ(test2, true);
    ASSERT_EQ(test3, true);
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}