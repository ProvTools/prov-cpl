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

CREATE TABLE IF NOT EXISTS cpl_objects (
       id BIGSERIAL,
       originator VARCHAR(255),
       name VARCHAR(255),
       type VARCHAR(100),
       creation_time TIMESTAMP DEFAULT NOW(),
       container_id BIGINT,
       session_id BIGINT,
       PRIMARY KEY(id),
       FOREIGN KEY(session_id)
                   REFERENCES cpl_sessions(id));

CREATE TABLE IF NOT EXISTS cpl_sessions (
       id BIGSERIAL,
       mac_address VARCHAR(18),
       username VARCHAR(255),
       pid INT,
       program VARCHAR(4095),
       cmdline VARCHAR(4095),
       initialization_time TIMESTAMP DEFAULT NOW(),
       PRIMARY KEY (id));

CREATE TABLE IF NOT EXISTS cpl_relations (
       id BIGSERIAL,
       from_id BIGSERIAL NOT NULL,
       to_id BIGSERIAL NOT NULL,
       type INT,
       container_id BIGINT,
       PRIMARY KEY(id),
       FOREIGN KEY(from_id)
                   REFERENCES cpl_objects(id)
                   ON DELETE CASCADE,
       FOREIGN KEY(to_id)
                   REFERENCES cpl_objects(id)
                   ON DELETE CASCADE,
       FOREIGN KEY(container_id)
                   REFERENCES cpl_objects(id)
                   ON DELETE CASCADE);

CREATE TABLE IF NOT EXISTS cpl_relation_properties (
      id BIGINT,
      name VARCHAR(255) NOT NULL,
      value VARCHAR(4095) NOT NULL,
      FOREIGN KEY(id)
            REFERENCES cpl_relations(id)
            ON DELETE CASCADE);

CREATE TABLE IF NOT EXISTS cpl_object_properties (
       id BIGINT,
       name VARCHAR(255) NOT NULL,
       value VARCHAR(4095) NOT NULL,
       FOREIGN KEY(id)
           REFERENCES cpl_objects(id)
           ON DELETE CASCADE);

ALTER TABLE cpl_objects ADD CONSTRAINT cpl_objects_fk
      FOREIGN KEY (container_id)
      REFERENCES cpl_objects(id)
      ON DELETE CASCADE;

ALTER SEQUENCE cpl_objects_id_seq RESTART WITH 1;

ALTER SEQUENCE cpl_sessions_id_seq RESTART WITH 1;

ALTER SEQUENCE cpl_relations_id_seq RESTART WITH 1;

CREATE OR REPLACE RULE cpl_relation_properties_ignore_duplicate_inserts AS
    ON INSERT TO cpl_relation_properties
   WHERE (EXISTS ( SELECT 1
           FROM cpl_relation_properties
          WHERE cpl_relation_properties.id = NEW.id 
          AND cpl_relation_properties.name = NEW.name)) 
      DO INSTEAD NOTHING;

CREATE OR REPLACE RULE cpl_object_properties_ignore_duplicate_inserts AS
    ON INSERT TO cpl_object_properties
   WHERE (EXISTS ( SELECT 1
           FROM cpl_object_properties
          WHERE cpl_object_properties.id = NEW.id 
          AND cpl_object_properties.name = NEW.name)) 
      DO INSTEAD NOTHING;

--
-- Grant the appropriate privileges
--

GRANT ALL PRIVILEGES ON TABLE cpl_objects TO cpl WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON TABLE cpl_sessions TO cpl WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON TABLE cpl_relations TO cpl WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON TABLE cpl_relation_properties TO cpl WITH GRANT OPTION;
GRANT ALL PRIVILEGES ON TABLE cpl_object_properties TO cpl WITH GRANT OPTION;

