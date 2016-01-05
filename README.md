# Core Provenance Library
Provenance is metadata that describes the history of a digital object: where it came from, how it came to be in its present state, who or what acted upon it, etc. It is especially important in computational science, where it enables the researches to precisely track how each document came into existence, provides a means to experimental reproducibility, and aids them in debugging what went wrong during a computation.

The adoption of provenance among computational scientists is low, because most existing systems require the users to adopt a particular tool set in order to benefit from their functionality, such as the requirement to use a particular programming language, operating system, or a workflow engine. Core Provenance Library (CPL) takes the opposite approach by enabling the scientists to easily integrate provenance collection to their existing tools.

Core Provenance Library is designed to run on a variety of platforms, work with multiple programming languages, and be able to use a several different database backends. An application would use the library's API to disclose its provenance by creating provenance objects and disclosing data and control flow between the objects. The library would take care of persistently storing the provenance, detecting and breaking the cycles, and providing an interface to query and visualize the collected provenance.

## Disclosing Provenance
A quick summary:

- Attach/detach from the database backend: `cpl_attach`/`cpl_detach`
- Create/lookup provenance objects: `cpl_create_object`/`cpl_lookup_object`
- Disclose data/control flow: `cpl_data_flow`/`cpl_control_flow`
- Add custom properties: `cpl_add_property`

## Accessing Provenance

- Using the built-in programmatic API
- Using the included "`cpl`" command-line tool (still work in progress)
- Visualization using [Provenance Map Orbiter](https://github.com/pmacko86/provenance-map-orbiter) or any other CPL-enabled visualization tool

## Supported Languages

- C/C++
- Java
- Perl
- Python

## Supported Database Backends
- Relational databases using ODBC (MySQL, PostgreSQL, etc.)
- Work in progress: RDF stores using the RDF/SPARQL protocol (4store)

## Publications
Peter Macko and Margo Seltzer. *A General-Purpose Provenance Library.* 4th Workshop on the Theory and Practice of Provenance (TaPP '12), Boston, MA, USA, June 2012. ([pdf](http://www.eecs.harvard.edu/~pmacko/papers/cpl-tapp12.pdf))
