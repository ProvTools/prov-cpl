# prov-CPL

Provenance is metadata that describes the history of a digital object: where it came from, how it came to be in its 
present state, who or what acted upon it, etc. It is especially important in computational science, where it enables 
the researches to precisely track how each document came into existence, provides a means to experimental reproducibility, 
and aids them in debugging what went wrong during a computation.

The adoption of provenance among computational scientists is low, because most existing systems require the users
to adopt a particular tool set in order to benefit from their functionality, such as the requirement to use a 
particular programming language, operating system, or a workflow engine. Core Provenance Library (CPL) takes the 
opposite approach by enabling the scientists to easily integrate provenance collection to their existing tools.

The Core Provenance Library is designed to run on a variety of platforms, work with multiple programming languages,
and be able to use a several different database backends. An application would use the library's API to disclose 
its provenance by creating provenance objects and disclosing data and control flow between the objects. The 
library would take care of persistently storing the provenance, detecting cycles, and providing 
an interface to query the collected provenance.

+An overview of the PROV standard can be found at: https://www.w3.org/TR/2013/NOTE-prov-overview-20130430/
+For specifics about JSON: https://www.w3.org/Submission/2013/SUBM-prov-json-20130424/
+

## Disclosing Provenance

Quick summary:
- Attach/detach from the database backend: `cpl_attach/cpl_detach`
- Create/lookup provenance: `cpl_create_[bundle/object/relation]/cpl_lookup_[...]`
- Add custom properties: `cpl_add_[...]_property`

Check test files for details.

## Accessing Provenance

Use the built-in programmatic API. Check test files for details.

## JSON Handling

Quick summary:
- Validate JSON document: `validate_json`
- Import JSON document: `import_document_json`
- Export bundle as JSON: `export_bundle_json`

Check test files for details.

## Supported Languages

* C/C++
* Java
* Python
* R

## Installation

Required:
* C/C++ Standalone
* ODBC Backend

Optional:
* Language specific bindings

### C/C++ Standalone

Required Packages:
- nlohmann_json
- boost

To compile and install CPL, please type the following commands:
```
make release
[sudo] (for linux) make install
```

The default installation directory is /usr/local.

To clean the compile, please use the `clean` or `distclean` make targets.
To uninstall, please use the `uninstall` target - separately for the main
CPL build and for the language-specific bindings.
### ODBC Backend

Required Packages:
- unixodbc
- unixodbc-dev
- odbcinst
- odbc-postgresql

#### Installing ODBC data sources

The easiest way to add a data source is to use ODBCConfig available from
the `unixodbc-bin` package. Alternatively, you can edit the `/etc/odbc.ini` file
manually. For example, if you use the PostgreSQL ODBC driver, the database is
running locally, and the database name is `cpl`, the appropriate configuration
entry would be:

      [CPL]
      Description     = PostgreSQL Core Provenance Library
      Driver          = PostgreSQL
      Server          = localhost
      Database        = cpl
      Port            = 
      Socket          = 
      Option          = 
      Stmt            = 
      User            = cpl
      Password        = cplcplcpl

You can verify that the data source is installed by running: `odbcinst -q -s`

You will also need to install the PostgreSQL ODBC driver from https://odbc.postgresql.org/.
This will require adding the following entry to `/usr/local/etc/odbcinst.ini`:

      [PostgreSQL]
      Description = PostgreSQL ODBC driver (Unicode version)
      Driver = psqlodbcw.so

You can verify that the data driver is installed by running: `odbcinst -q -d`

#### Configuring PostgreSQL

PostgreSQL might not work out of the box on Ubuntu. If you experience user
authentication issues for the user cpl, please add the following line to your
`pg_hba.conf` file:

    local   cpl             cpl                                     md5

Make sure to add it just before the line:

    local   all             all                                     peer


If you are able to connect to the database using psql (i.e. `psql cpl -U cpl`),
but you cannot open an ODBC connection, you might need to change the location
of the PostgreSQL socket to `/tmp` by setting the following property in your
postgresql.conf file and restarting the PostgreSQL service:

     unix_socket_directory = '/tmp'

Finally, run the configuration script `../../scripts/postgresql-setup.sql` 
as the user postgres. (Note: if you've used homebrew to install postgres, then 
your default username may instead be root.) On Linux and Unix systems, this can
be easily achieved by cd-ing into CPL's main project directory and running from 
the command line:

     sudo psql -U postgres postgres < scripts/postgresql-setup.sql

This will create user `cpl` with password `cplcplcpl`, database `cpl`, and its
corresponding schema.

### Java Bindings

Required Packages:
* default-jdk
* swig

If you plan to use the CPL bindings via Maven, please do the following:
```
cd bindings/java  (if necessary)
make release
sudo make -C CPLDirect install
make -C CPL maven-install
```
The Maven group ID is `edu.harvard.pass`, and the artifact ID is `cpl`. You can
find the most recent version of the project by examining `CPL/Makefile`.

To use the bindings by manually including the .jar or to use with Ant:
```
cd bindings/java (if necessary)
make release
sudo make install
```
This will (among other things) create `/usr/local/java/CPL.jar`, which you can
then include in your project. You do not need to include `CPLDirect.jar`, since
it is statically included in CPL.jar.

In both cases, you will also need to move `libCPLDirect-java.dylib` into your `java.library.path` 
or change your `java.library.path` to include `/usr/local/lib`.

Note: If `make release` fails with the "Permission denied" error, please run
`sudo make distclean` first. If `make release` fails mysteriously, perhaps
`sudo updatedb` will help.

### Python Bindings

Required Packages:
* python-dev
* swig

To compile and install the CPL python bindings, please do the following:  
```
cd bindings/python
make release
[sudo] make install
```
  
### R Bindings

Required Packages:
* Rcpp

To compile and install the CPL R bindings, please do the following: 
```
cd bindings/r/CPL  
(From inside R:)
  > library(Rcpp)
  > compileAttributes()
cd ..  
R CMD build CPL 
R CMD install CPL_3.0.tar.gz
```
The bindings currently only implement basic JSON document handling.

## To-Dos

* Expand R bindings to full library support
* Add MySQL and RDF database backends
* Improve querying with CypherToSQL tool




# About the Library
A breif overview of the main modules and API functions for the purpose of the fake news provenance project.


## Modules

### cpl-standalone
This is the real "meat" of prov-cpl, and contains the API functions for interacting with the database. These functions almost all query an underlying backend function found in cpl/odbc. 

### backends/cpl-odbc
This modlule contains backend functions that are invoked by the API in cpl-standalone and serve to interact with and modify the database through prepared SQL statements.

### bindings
Includes the modules that "bind" cpl-standalone to a Python/Java/R interface by providing language-specific wrappers to the API functions. The only time you will need to modify this is if you change the signature of a function in cpl-standalone such that it needs to be called differently by the application. It's wise to this as sparingly as possible.

### include
Contains most of the header files for this project. If you make any changes to an existing prov-cpl function, you may need to change its definition in some of: cpl.h, cpl-db-backend.h, cplxx.h. If your compilation is failing with type errors, look here first. I recommend changing only what you absolutely have to in order to compile the project.

### make
This directory holds some of the root makefiles for the project. You shouldn't have to worry about it unless your build of prov-cpl is failing.

### scripts
Contains some shell scripts used to configure the PSQL backend. The one specific to the database spec for this project is scripts/postgresql-setup.sql

### test, rpm, rest-service
I haven't used these at all, other than to edit a few test functions to use new API function signatures. You probably won't need to open or worry about these directories.


## API functions

It's important to understand that pretty much every API function works on the same few layers. As an example, if you made a call to create_object in your python code, that would call the python binding for create_object which would in turn invoke the cpl_create_object function in cpl-standalone. cpl_create_object would call cpl_db_create_object in backends/cpl-odbc, which would execute a prepared create_object statement (found in the same file) on the database to insert the specified object. On every level there are input validatation checks. Keeping in mind that most of these functions are the same wrapper around a different database action, here's a really breif description of the main ones you'll use. These can be found in cpl-standalone.cpp and are all called by the language-specific bindings.

### cpl_create_object
Creates an object in the database with: 
- a prefix (same thing as originator, basically a reference to what experiment this object belongs to, passed in by the user)
- a name, which can be anything but should ideally be somewhat unique
- a type, which is an number indicating whether this is an entity, activity, agent, or bundle (defined in cpl.h)
- In this version of prov-cpl the bundle-id parameter is not inserted into the objects database, as objects no longer belong one-to-one with a bundle. Rather, a different one-to-many relationship between objects and bundles is established in the cpl_relations table as a result of this call.
out_id is used as a way to get the generated ID back from the database. It's internal to the API and the application doesn't need to interact with it. At the top level, this function will return the ID of the created object, or an error value if the call was unsuccessful.

### cpl_lookup_object
Takes the same parameters as cpl_create_object, returns the object ID if it exists and an error otherwise. It's always a good idea to try to look up an object first before creating a new one. That way, if we're analyzing a new paper and there's already an "Author: Margo Seltzer" object in the database because we've previously analyzed another one of her articles, we'll connect the same Margo Seltzer to both article groupings. Running create without lookup will create two Margo Seltzers in the database: one who wrote the first article, and another who wrote the second.

### cpl_lookup_or_create_object
Same parameters as the lookup and create functions; automatically creates the specified object if it fails on lookup. 

### cpl_add_object_property 
Adds a property to an object by modifying the cpl_object_properties table in the database. Requires a key and a value. Properties are a good way to make objects unique, or to include additional metadata about an object without conforming to the prov-json specification. As an example, if I wanted to add a property to my Margo Seltzer object, my key could be "pets" and value "cat named sushi". 

### cpl_lookup_object_by_property
Takes a key and value and returns an object with the matching property. If I wanted to find an author with a cat named sushi, I could do a lookup_by_property call and find out that Margo Seltzer matches that criteria. If you've created multiple objects with the same name but added different properties, as I have done with having multiple articles with the same object name but different URLs added as properties, then you should use this call instead of the regular lookup.

### cpl_add_relation
Modifies the cpl_relations database table and adds a relation from one object to another. Relations are one of several types, as defined in cpl.h. In this version of the project there is an "InBundle" relationship type (19) that represents an objects membership to a bundle, and consequently, that bundle's ownership of the object. Each relation type is defined between specific object types. Only an entity and an agent can have the "wasAttributedTo" relationship. The "InBundle" relationship is defined between a bundle and an entity, agent, or activity. The relations enum can be found in cpl_standalone.cpp.
- Note: As of right now the "in bundle" relationship must be established by a call to add_relation after create_object, but it could easily be bundled with create_object as the bundle ID must be supplied anyway. However, when an object is found by lookup, a second call to add_relation is absolutely necessary to place the existing object within an additional bundle grouping.

### cpl_add_relation_property
Similar to add_object_property, takes a key and value and associates it with a specific relation id in the cpl_relation_properties table

### cpl_get_relation_properties
Analogous to get_object_properties

### cpl_create_bundle
Since bundles are just objects in this prov-cpl version, this is just a wrapper around create_object that creates an object of type "bundle" and returns a "bundle id" to the application, such that this object can be treated the same as as a bundle from previous versions (minimal change to the language-specific bindings).

### cpl_lookup_bundle
Again, this is just a wrapper around cpl_lookup_object and serves to maintain the API despite the fact that bundles all live in the objects database table.

### cpl_add_bundle_property
A wrapper around add_object_property for adding properties to a bundle. This could be used to associate a bundle ID with information about an article.

### cpl_get_bundle_properties
A wrapper around get_object_properties.

### cpl_get_all_objects
WARNING! This function does something different in this version. It used to return all objects in the database, but has been rewritten so that it returns only the *bundles* belonging to a specific originator (the prefix parameter). It is used in the case when you want a single graph representing multiple bundles/articles. Example: If you want to represent 4 articles by Margo Seltzer in the same graph, analyze them all using a specific prefix value (i.e. "margo-article-group") and then fetch all associated bundles using get_all_objects and use the *list* of bundles to generate one prov json file for the grouping, which can then be graphed. 

### export_bundle_json
This is the function that's used to generate prov JSON strings that represent a bundle or group of bundles. It takes an array of bundle IDs and returns a string representing the objects and relations belonging to those bundles. 

### cpl_get_bundle_objects 
Fetches all objects belonging to a bundle, using an SQL statement that finds the bundle's relations and uses them to search the object table. This is pretty much only called by export_bundle_json.

### cpl_get_bundle_relations
Gets all relationships that are designated as belonging to a specific bundle. This includes relationships between objects within a bundle, not just the relationships between the objects and the bundle itself. As an example, if my bundle represents an article in which author "Margo Seltzer" said the quote "I have a cat named sushi", then the relationship between the author and quote object would belong to my bundle. Like get_bundle_objects, this function is really only used internally by export_bundle_json to generate the prov JSON file.

### import_document_json
This function takes a prov JSON document, processes and validates it, and loads the represented objects into the database. I have not tested this function and I'm not totally sure if it's compatible with this branch and database version. Probably worth checking out soon.






