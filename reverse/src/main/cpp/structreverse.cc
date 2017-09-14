#include <stdio.h>
#include <malloc.h>
#include <jni.h>
#include "hellojni.h"
#include "my_log.h"
#include "syscallstack.h"

// our structures
// ==============
// information about our customers

#define PC 0x1 // 0x01
#define MAC 0x2 // 0x02
#define WINDOWS 0x1 // 0x04
#define DOS 0x2 // 0x08
#define OS_X 0x4 // 0x10
#define DISASSEMBLY 0x1 // 0x20
#define RECOVERY 0x2 // 0x40
#define CRYPTOGRAPHY 0x3 // 0x60

#define PRODUCTS_COUNT 4

// generic products we're selling
enum product_category_t { // an enumerated type
    BOOK,
    SOFTWARE,
    HARDWARE // we actually don't sell hardware
};

struct customer_t { // a typical structure
    long id;
    char name[30];
    char sex; // 'm'ale - 'f'emale
};
// we sell books
struct book_t {
    char title[128]; // an ASCII string
};

// and we sell computer softwares
struct software_info_t { // a structure containing various bitfields
    unsigned int plateform : 2; // 2 bits reserved for the plateform -
// plateforms can be combined (0x03)
    unsigned int os : 3; // 3 bits reserved for the OS -
// OS can be combined (0x1C)
    unsigned int category : 2; // 2 bits reserved for the category -
// categories can't be combined (0x60)
};
struct software_t {
    software_info_t info;
    char name[32];
};

struct softwares_t
{
    //a variable length structure
    long count;
    software_t softs[3];
};

union product_u { // an union to contain product information
// depending on its category
    book_t book;
    software_t software;
// struct hardware_t hardware; // we actually don't sell hardware
};
struct product_t { // a structure containing another structure
    long id;
    product_category_t category;
    product_u p;
};

// our data
// ========
// our customers
static customer_t customers[] = { // an initialized array to memorize our customers
        { 1, "Peter", 'm' },
        { 2, "John", 'm' },
        { 3, "Mary", 'f' },
        { 0 }
};
// our products
static book_t ida_book = { "IDA QuickStart Guide" };

//software_t software1 =

static softwares_t softwares = // an initialized variable length structure
        {
                3,
                {
                        { { PC, WINDOWS|DOS, DISASSEMBLY }, "IDA Pro" },
                        { { PC|MAC, WINDOWS|OS_X, RECOVERY }, "PhotoRescue" },
                        { { PC, WINDOWS, CRYPTOGRAPHY }, "aCrypt" }
                }
        };

// our functions
// =============
// check software information
static int check_software(software_info_t software_info)
{
    bool valid = true;
    if (software_info.plateform & PC)
    {
        if (! (software_info.plateform & MAC) && (software_info.os & OS_X))
            valid = false; // OS-X isn't yet available on PC ;)
    }
    else if (software_info.plateform & MAC)
    {
        if (! (software_info.plateform & PC) && ((software_info.os & WINDOWS) ||
                                                 (software_info.os & DOS)))
            valid = false; // Windows & DOS aren't available on Mac...
    }
    else
        valid = false;
    return valid;
}
// check product category
static int check_product(product_category_t product_category)
{
    bool valid = true;
    if (product_category == HARDWARE)
    {
        valid = false;
        LOGI("We don't sell hardware for the moment...\n");
    }
    return valid;
}
// print customer information
static void print_customer(customer_t *customer)
{
    sys_call_stack();
    LOGI("CUSTOMER %04X: %s (%c)\n", customer->id, customer->name, customer->sex);
}
// print book information
static void print_book(book_t *book)
{
    LOGI("BOOK: %s\n", book->title);
}
// print software information
static void print_software(software_t *software)
{
    LOGI("SOFTWARE: %s:", software->name);
// plateform
// we use 'if', as plateforms can be combined
    if (software->info.plateform & PC)
        LOGI(" PC");
    if (software->info.plateform & MAC)
        LOGI(" MAC");
    LOGI(";");
// OS
// we use 'if', as os can be combined
    if (software->info.os & WINDOWS)
        LOGI(" WINDOWS");
    if (software->info.os & DOS)
        LOGI(" DOS");
    if (software->info.os & OS_X)
        LOGI(" OS-X");
    LOGI(";");
// category
// we use 'switch', as categories can't be combined
    switch(software->info.category)
    {
        case DISASSEMBLY:
            LOGI(" DISASSEMBLY");
            break;
        case RECOVERY:
            LOGI(" RECOVERY");
            break;
        case CRYPTOGRAPHY:
            LOGI(" CRYPTOGRAPHY");
            break;
    }
    LOGI("\n");
}
// print product information
static bool print_product(product_t *product)
{
    if (! check_product(product->category))
        return false;
    LOGI("PRODUCT %04X: ", product->id);
    switch(product->category) {
        case BOOK:
            print_book(&product->p.book);
            break;
        case SOFTWARE:
            print_software(&product->p.software);
            break;
    }
    return true;
}

static void nativeStructRevers()
{
    // print customers listing
    LOGI("CUSTOMERS:\n");
    customer_t *customer = customers;
    while (customer->id != 0)
    {
        print_customer(customer);
        customer++;
    }
// allocate a small array to store our products in memory
    int ilen = sizeof(product_t);
    product_t *products = (product_t*) malloc(PRODUCTS_COUNT * sizeof(product_t));
// insert our products
    products[0].id = 1;
    products[0].category = BOOK;
    products[0].p.book = ida_book;
    products[1].id = 2;
    products[1].category = SOFTWARE;
    products[1].p.software = softwares.softs[0]; // we insert softwares from our
// variable length structure
    products[2].id = 3;
    products[2].category = SOFTWARE;
    products[2].p.software = softwares.softs[1];
    products[3].id = 4;
    products[3].category = SOFTWARE;
    products[3].p.software = softwares.softs[2];
// verify and print each product
    LOGI("\nPRODUCTS:\n");
    for (int i = 0; i < PRODUCTS_COUNT; i++)
    {
// check validity of the product category
        if (! check_product(products[i].category))
        {
            LOGI("Invalid product !!!\n");
            break;
        }
// check validity of softwares
        if (products[i].category == SOFTWARE)
        {
            if (! check_software(products[i].p.software.info))
            {
                LOGI("Invalid software !!!\n");
                break;
            }
        }
// and print the product
        print_product(&products[i]);
    }
    free(products);
}

// our main program
// ================
JNIEXPORT void JNICALL Java_com_reverse_HelloJni_nativeStructRevers(JNIEnv* env, jobject thiz)
{
    nativeStructRevers();
}
