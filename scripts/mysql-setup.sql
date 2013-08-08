--
-- mysql-setup.sql
-- Core Provenance Library
--
-- Copyright 2011
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
-- Contributor(s): Peter Macko
--


-- ------------------------------------------------------------------------ --
-- Instructions                                                             --
-- ------------------------------------------------------------------------ --
--
-- Execute this script as the MySQL root.
--
-- Usage on Linux:
--    mysql -u root -p < scripts/mysql-setup.sql
--


-- ------------------------------------------------------------------------ --
-- MySQL Setup                                                              --
-- ------------------------------------------------------------------------ --

--
-- Create the database and the default user with the default password
-- 

CREATE DATABASE IF NOT EXISTS cpl;
GRANT ALL PRIVILEGES ON cpl.* 
      TO 'cpl'@'localhost' IDENTIFIED BY 'cplcplcpl'
      WITH GRANT OPTION;

USE cpl;


--
-- Create the schema
--

SET FOREIGN_KEY_CHECKS = 0;

CREATE TABLE IF NOT EXISTS cpl_objects (
       id_hi BIGINT,
       id_lo BIGINT,
       originator VARCHAR(255),
       name VARCHAR(255),
       type VARCHAR(100),
       creation_time TIMESTAMP DEFAULT NOW(),
       container_id_hi BIGINT,
       container_id_lo BIGINT,
       container_ver INT,
       PRIMARY KEY (id_hi, id_lo),
       FOREIGN KEY (container_id_hi, container_id_lo, container_ver)
                    REFERENCES cpl_versions(id_hi, id_lo, version));

CREATE TABLE IF NOT EXISTS cpl_sessions (
       id_hi BIGINT,
       id_lo BIGINT,
       mac_address VARCHAR(18),
       username VARCHAR(255),
       pid INT,
       program VARCHAR(4095),
       cmdline VARCHAR(4095),
       initialization_time TIMESTAMP DEFAULT NOW(),
       PRIMARY KEY (id_hi, id_lo));

CREATE TABLE IF NOT EXISTS cpl_versions (
       id_hi BIGINT,
       id_lo BIGINT,
       version INT,
       creation_time TIMESTAMP DEFAULT NOW(),
       session_id_hi BIGINT,
       session_id_lo BIGINT,
       PRIMARY KEY(id_hi, id_lo, version),
       FOREIGN KEY(id_hi, id_lo) REFERENCES cpl_objects(id_hi, id_lo),
       FOREIGN KEY(session_id_hi, session_id_lo)
                   REFERENCES cpl_sessions(id_hi, id_lo));

CREATE TABLE IF NOT EXISTS cpl_ancestry (
       from_id_hi BIGINT NOT NULL,
       from_id_lo BIGINT NOT NULL,
       from_version INT NOT NULL,
       to_id_hi BIGINT NOT NULL,
       to_id_lo BIGINT NOT NULL,
       to_version INT NOT NULL,
       type INT,
       PRIMARY KEY(from_id_hi, from_id_lo, from_version,
                   to_id_hi, to_id_lo, to_version),
       FOREIGN KEY(from_id_hi, from_id_lo, from_version)
                   REFERENCES cpl_versions(id_hi, id_lo, version),
       FOREIGN KEY(to_id_hi, to_id_lo, to_version)
                   REFERENCES cpl_versions(id_hi, id_lo, version));

CREATE TABLE IF NOT EXISTS cpl_properties (
       id_hi BIGINT,
       id_lo BIGINT,
       version INT,
       name VARCHAR(255) NOT NULL,
       value VARCHAR(4095) NOT NULL,
	   PRIMARY KEY(id_hi, id_lo, version, name),
       FOREIGN KEY(id_hi, id_lo, version)
           REFERENCES cpl_versions(id_hi, id_lo, version));

SET FOREIGN_KEY_CHECKS = 1;

