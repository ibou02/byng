--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET lock_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;

--
-- Name: plpgsql; Type: EXTENSION; Schema: -; Owner: 
--

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION plpgsql; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';


SET search_path = public, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: crawled_sites; Type: TABLE; Schema: public; Owner: postgres; Tablespace: 
--

CREATE TABLE crawled_sites (
    url text NOT NULL,
    title text,
    favicon text,
    h1 text[],
    h2 text[],
    h3 text[],
    h4 text[],
    h5 text[],
    h6 text[],
    paragraphes text[],
    crawled_at timestamp without time zone,
    crawled_for timestamp without time zone
);


ALTER TABLE public.crawled_sites OWNER TO byng;

--
-- Name: unique_url; Type: CONSTRAINT; Schema: public; Owner: byng; Tablespace: 
--

ALTER TABLE ONLY crawled_sites
    ADD CONSTRAINT unique_url PRIMARY KEY (url);


--
-- Name: public; Type: ACL; Schema: -; Owner: byng
--

REVOKE ALL ON SCHEMA public FROM PUBLIC;
REVOKE ALL ON SCHEMA public FROM byng;
GRANT ALL ON SCHEMA public TO byng;
GRANT ALL ON SCHEMA public TO PUBLIC;


--
-- PostgreSQL database dump complete
--

