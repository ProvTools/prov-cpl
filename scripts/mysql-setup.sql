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

CREATE TABLE IF NOT EXISTS cpl_objects (
       id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
       originator VARCHAR(255),
       name VARCHAR(255),
       type VARCHAR(100),
       container_id INT,
       container_ver INT);

CREATE TABLE IF NOT EXISTS cpl_sessions (
       id INT NOT NULL AUTO_INCREMENT PRIMARY KEY,
       user VARCHAR(255),
       pid INT,
       program VARCHAR(4096),
       initialization_time TIMESTAMP DEFAULT NOW());

CREATE TABLE IF NOT EXISTS cpl_versions (
       id INT NOT NULL REFERENCES cpl_objects(id),
       version INT,
       creation_time TIMESTAMP DEFAULT NOW(),
       session INT REFERENCES cpl_sessions(id),
       PRIMARY KEY(id, version));

CREATE TABLE IF NOT EXISTS cpl_ancestry (
       from_id INT NOT NULL,
       from_version INT,
       to_id INT NOT NULL,
       to_version INT,
       type INT,
       PRIMARY KEY(from_id, from_version, to_id, to_version),
       FOREIGN KEY(from_id, from_version) REFERENCES cpl_versions(id, version),
       FOREIGN KEY(to_id, to_version) REFERENCES cpl_versions(id, version));

ALTER TABLE cpl_objects ADD CONSTRAINT
      FOREIGN KEY (container_id, container_ver)
      REFERENCES cpl_versions(id, version);
