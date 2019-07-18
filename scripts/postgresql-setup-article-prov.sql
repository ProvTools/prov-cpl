--
-- postgresql-setup.sql
-- Prov-CPL
--
-- Copyright 2016
--      The President and Fellows of Harvard College.
--
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions
-- are met:
-- 1. Redistributions of source code must retain the above copyright
--    notice, this list of conditions and the following disclaimer.
-- 2. Redistributions in binary form must reproduce the above copyright
--    notice, this list of conditions and the following disclaimer in the
--    documentation and/or other materials provided with the distribution.
-- 3. Neither the name of the University nor the names of its contributors
--    may be used to endorse or promote products derived from this software
--    without specific prior written permission.
--
-- THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
-- ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
-- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
-- ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
-- FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
-- DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
-- OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
-- HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
-- LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
-- OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
-- SUCH DAMAGE.
--
-- Contributor(s): Jackson Okuhn, Peter Macko
--


-- ------------------------------------------------------------------------ --
-- Instructions                                                             --
-- ------------------------------------------------------------------------ --
--
-- Execute this script as the user postgres.
--
-- Usage on Linux:
--   sudo -u postgres psql postgres < scripts/postgresql-setup.sql
--


-- ------------------------------------------------------------------------ --
-- PostgreSQL Setup                                                         --
-- ------------------------------------------------------------------------ --

--
-- Create the database and the default user with the default password
-- 

CREATE DATABASE cpl;
CREATE USER cpl WITH PASSWORD 'cplcplcpl';
GRANT ALL PRIVILEGES ON DATABASE cpl TO cpl WITH GRANT OPTION;

\connect cpl


--
-- Create the schema
--
CREATE TABLE IF NOT EXISTS cpl_sessions (
       id BIGSERIAL,
       mac_address VARCHAR(18),
       username VARCHAR(255),
       pid INT,
       program VARCHAR(4095),
       cmdline VARCHAR(4095),
       initialization_time TIMESTAMP DEFAULT NOW(),
       PRIMARY KEY (id));

CREATE TABLE IF NOT EXISTS cpl_bundles (
        id BIGSERIAL,
       prefix VARCHAR(255),
       name VARCHAR(255),
       type INT,
       creation_time TIMESTAMP DEFAULT NOW(),
       PRIMARY KEY(id));

CREATE TABLE IF NOT EXISTS cpl_objects (
       id BIGSERIAL,
       prefix VARCHAR(255),
       name VARCHAR(255),
       type INT,
       creation_time TIMESTAMP DEFAULT NOW(),
       PRIMARY KEY(id));

CREATE TABLE IF NOT EXISTS cpl_relations (
       id BIGSERIAL,
       from_id BIGINT,
       to_id BIGINT,
       type INT,
       PRIMARY KEY(id));

CREATE TABLE IF NOT EXISTS cpl_prefixes (
      id BIGINT,
      prefix VARCHAR(255) NOT NULL,
      iri VARCHAR(4095) NOT NULL,
      FOREIGN KEY(id)
            REFERENCES cpl_objects(id)
            ON DELETE CASCADE);

CREATE TABLE IF NOT EXISTS cpl_bundle_properties (
      id BIGINT,
      prefix VARCHAR(255) NOT NULL,
      name VARCHAR(255) NOT NULL,
      value VARCHAR(4095) NOT NULL,
      FOREIGN KEY(id)
            REFERENCES cpl_bundles(id)
            ON DELETE CASCADE);

CREATE TABLE IF NOT EXISTS cpl_relation_properties (
      id BIGINT,
      prefix VARCHAR(255) NOT NULL,
      name VARCHAR(255) NOT NULL,
      value VARCHAR(4095) NOT NULL,
      FOREIGN KEY(id)
            REFERENCES cpl_relations(id)
            ON DELETE CASCADE);

CREATE TABLE IF NOT EXISTS cpl_object_properties (
       id BIGINT,
       prefix VARCHAR(255) NOT NULL,
       name VARCHAR(255) NOT NULL,
       value VARCHAR(4095) NOT NULL,
       FOREIGN KEY(id)
           REFERENCES cpl_objects(id)
           ON DELETE CASCADE);

INSERT INTO cpl_sessions (id, mac_address, username, pid, program, cmdline)
  VALUES (0, NULL, NULL, NULL, NULL, NULL);
INSERT INTO cpl_bundles (id, prefix, name, type)
  VALUES (0, NULL, NULL, NULL);
INSERT INTO cpl_objects (id, prefix, name, type)
  VALUES (0, NULL, NULL, NULL);
INSERT INTO cpl_relations (id, from_id, to_id, type)
  VALUES (0, NULL, NULL, NULL);

CREATE OR REPLACE RULE cpl_relation_properties_ignore_duplicate_inserts AS
    ON INSERT TO cpl_relation_properties
   WHERE (EXISTS ( SELECT 1
           FROM cpl_relation_properties
          WHERE cpl_relation_properties.id = NEW.id 
          AND cpl_relation_properties.prefix = NEW.prefix
          AND cpl_relation_properties.name = NEW.name)) 
      DO INSTEAD NOTHING;

CREATE OR REPLACE RULE cpl_object_properties_ignore_duplicate_inserts AS
    ON INSERT TO cpl_object_properties
   WHERE (EXISTS ( SELECT 1
           FROM cpl_object_properties
          WHERE cpl_object_properties.id = NEW.id 
          AND cpl_object_properties.prefix = NEW.prefix
          AND cpl_object_properties.name = NEW.name)) 
      DO INSTEAD NOTHING;

CREATE OR REPLACE RULE cpl_bundle_properties_ignore_duplicate_inserts AS
    ON INSERT TO cpl_bundle_properties
   WHERE (EXISTS ( SELECT 1
           FROM cpl_bundle_properties
          WHERE cpl_bundle_properties.id = NEW.id
          AND cpl_bundle_properties.prefix = NEW.prefix
          AND cpl_bundle_properties.name = NEW.name)) 
      DO INSTEAD NOTHING;

CREATE OR REPLACE RULE cpl_prefixes_ignore_duplicate_inserts AS
    ON INSERT TO cpl_prefixes
   WHERE (EXISTS ( SELECT 1
           FROM cpl_prefixes
          WHERE cpl_prefixes.id = NEW.id 
          AND cpl_prefixes.prefix = NEW.prefix)) 
      DO INSTEAD NOTHING;

-- TODO add empty prefix conversion rule

--
-- Grant the appropriate privileges
--
GRANT ALL PRIVILEGES ON TABLE cpl_sessions TO cpl WITH GRANT OPTION; 
GRANT ALL PRIVILEGES ON TABLE cpl_bundles TO cpl WITH GRANT OPTION; 
GRANT ALL PRIVILEGES ON TABLE cpl_objects TO cpl WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON TABLE cpl_relations TO cpl WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON TABLE cpl_relation_properties TO cpl WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON TABLE cpl_object_properties TO cpl WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON TABLE cpl_bundle_properties TO cpl WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON TABLE cpl_prefixes TO cpl WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON SEQUENCE cpl_objects_id_seq TO cpl WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON SEQUENCE cpl_bundles_id_seq TO cpl WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON SEQUENCE cpl_sessions_id_seq TO cpl WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON SEQUENCE cpl_relations_id_seq TO cpl WITH GRANT OPTION;
