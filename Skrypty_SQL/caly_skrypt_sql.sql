CREATE SCHEMA gospodarka;
SET search_path TO gospodarka;

-- ===============================
-- 1. UZYTKOWNICY
-- ===============================
CREATE TABLE Uzytkownik (
    id_uzytkownika SERIAL PRIMARY KEY,
    login VARCHAR(50) UNIQUE NOT NULL,
    haslo VARCHAR(255) NOT NULL,
    rola VARCHAR(20) NOT NULL CHECK (rola IN ('admin', 'rolnik', 'pracownik')),
    aktywny BOOLEAN DEFAULT TRUE
);

-- ===============================
-- 2. PRACOWNICY (DANE OPERACYJNE)
-- ===============================
CREATE TABLE Pracownik (
    id_pracownika SERIAL PRIMARY KEY,
    id_uzytkownika INT UNIQUE NOT NULL REFERENCES Uzytkownik(id_uzytkownika) ON DELETE CASCADE,
    id_rolnika INT REFERENCES Uzytkownik(id_uzytkownika) ON DELETE SET NULL,
    imie VARCHAR(50) NOT NULL,
    nazwisko VARCHAR(50) NOT NULL,
    stanowisko VARCHAR(50),
    telefon VARCHAR(20),
    data_zatrudnienia DATE
);

-- ===============================
-- 3. POLA I UPRAWY
-- ===============================
CREATE TABLE Pole (
    id_pola SERIAL PRIMARY KEY,
    nazwa VARCHAR(50) NOT NULL,
    powierzchnia DECIMAL(10,2),
    id_rolnika INT REFERENCES Uzytkownik(id_uzytkownika) ON DELETE CASCADE
);

CREATE TABLE Uprawa (
    id_uprawy SERIAL PRIMARY KEY,
    id_pola INT REFERENCES Pole(id_pola) ON DELETE CASCADE,
    nazwa VARCHAR(50) NOT NULL,
    data_siewu DATE,
    data_zbioru DATE
);

-- ===============================
-- 4. BUDYNKI
-- ===============================
CREATE TABLE Budynek (
    id_budynku SERIAL PRIMARY KEY,
    nazwa VARCHAR(50),
    typ VARCHAR(50),
    pojemnosc INT,
    id_rolnika INT REFERENCES Uzytkownik(id_uzytkownika) ON DELETE CASCADE
);

-- ===============================
-- 5. ZWIERZĘTA
-- ===============================
CREATE TABLE Zwierze (
    id_zwierzecia SERIAL PRIMARY KEY,
    gatunek VARCHAR(50),
    rasa VARCHAR(50),
    ilosc INT NOT NULL CHECK (ilosc >= 0), 
    id_budynku INT REFERENCES Budynek(id_budynku) ON DELETE SET NULL,
    id_rolnika INT REFERENCES Uzytkownik(id_uzytkownika) ON DELETE CASCADE
);

-- ===============================
-- 6. MASZYNY
-- ===============================
CREATE TABLE Maszyna (
    id_maszyny SERIAL PRIMARY KEY,
    nazwa VARCHAR(50) NOT NULL,
    typ VARCHAR(50),
    data_przegladu DATE,
    id_rolnika INT REFERENCES Uzytkownik(id_uzytkownika) ON DELETE SET NULL,
    id_budynku INT REFERENCES Budynek(id_budynku) ON DELETE SET NULL
);

-- ===============================
-- 7. MAGAZYN I SUROWCE
-- ===============================
CREATE TABLE Magazyn (
    id_magazynu SERIAL PRIMARY KEY,
    nazwa VARCHAR(50),
    id_rolnika INT REFERENCES Uzytkownik(id_uzytkownika) ON DELETE CASCADE
);

CREATE TABLE Surowiec (
    id_surowca SERIAL PRIMARY KEY,
    id_magazynu INT REFERENCES Magazyn(id_magazynu) ON DELETE CASCADE,
    nazwa VARCHAR(50),
    ilosc DECIMAL(10,2),
    kategoria VARCHAR(50)
);

-- ===============================
-- 8. PLONY (ROŚLINNE I ZWIERZĘCE)
-- ===============================
CREATE TABLE Plon (
    id_plonu SERIAL PRIMARY KEY,
    id_magazynu INT REFERENCES Magazyn(id_magazynu) ON DELETE CASCADE,
    id_uprawy INT REFERENCES Uprawa(id_uprawy) ON DELETE SET NULL,
    id_zwierzecia INT REFERENCES Zwierze(id_zwierzecia) ON DELETE SET NULL,
    nazwa VARCHAR(50) NOT NULL,
    ilosc DECIMAL(10,2),
    data_pozyskania DATE
);

-- ===============================
-- 9. ZADANIA
-- ===============================
CREATE TABLE Zadanie (
    id_zadania SERIAL PRIMARY KEY,
    nazwa VARCHAR(100) NOT NULL,
    opis TEXT,
    id_rolnika INT REFERENCES Uzytkownik(id_uzytkownika) ON DELETE CASCADE,
    id_pracownika INT REFERENCES Pracownik(id_pracownika) ON DELETE SET NULL,
    id_uprawy INT REFERENCES Uprawa(id_uprawy) ON DELETE SET NULL,
    id_maszyny INT REFERENCES Maszyna(id_maszyny) ON DELETE SET NULL,
    id_budynku INT REFERENCES Budynek(id_budynku) ON DELETE SET NULL,
    id_zwierzecia INT REFERENCES Zwierze(id_zwierzecia) ON DELETE SET NULL,
    id_pola INT REFERENCES Pole(id_pola) ON DELETE SET NULL,
    data_start DATE,
    data_koniec DATE,
    status VARCHAR(20) CHECK (status IN ('planowane','w_trakcie','wykonane', 'anulowane')) DEFAULT 'planowane'
);

-- ===============================
-- 10. RELACJE N-M: ZUŻYCIE SUROWCÓW W ZADANIACH
-- ===============================
CREATE TABLE Zadanie_Surowiec (
    id_zadania INT REFERENCES Zadanie(id_zadania) ON DELETE CASCADE,
    id_surowca INT REFERENCES Surowiec(id_surowca) ON DELETE CASCADE,
    ilosc_uzyta DECIMAL(10,2),
    PRIMARY KEY (id_zadania, id_surowca)
);

-- ===============================
-- 11. RELACJA N-M: PLONY POWIĄZANE Z ZADANIAMI 
-- ===============================
CREATE TABLE Zadanie_Plon (
    id_zadania INT REFERENCES Zadanie(id_zadania) ON DELETE CASCADE,
    id_plonu INT REFERENCES Plon(id_plonu) ON DELETE CASCADE,
    ilosc_pozyskana DECIMAL(10,2),
    PRIMARY KEY (id_zadania, id_plonu)
);

-- ===============================
-- 12. FINANSE
-- ===============================
CREATE TABLE Transakcja (
    id_transakcji SERIAL PRIMARY KEY,
    id_rolnika INT NOT NULL REFERENCES Uzytkownik(id_uzytkownika) ON DELETE CASCADE,
    typ VARCHAR(10) NOT NULL CHECK (typ IN ('kupno','sprzedaz')),
    data_transakcji TIMESTAMP NOT NULL DEFAULT NOW(),
    opis TEXT,
    kwota DECIMAL(12,2) NOT NULL CHECK (kwota >= 0)
);

-- ===============================
-- 13. RELACJA N-M: SUROWIEC POWIĄZANE Z TRANZAKCJĄ 
-- ===============================

CREATE TABLE Transakcja_Surowiec (
    id_transakcji INT REFERENCES Transakcja(id_transakcji) ON DELETE CASCADE,
    id_surowca INT REFERENCES Surowiec(id_surowca) ON DELETE CASCADE,
    ilosc DECIMAL(10,2) NOT NULL CHECK (ilosc > 0),
    cena_jednostkowa DECIMAL(12,2) NOT NULL CHECK (cena_jednostkowa >= 0),

    PRIMARY KEY (id_transakcji, id_surowca)
);

-- ===============================
-- 14. RELACJA N-M: PLONY POWIĄZANE Z TRANZKACJĄ 
-- ===============================

CREATE TABLE Transakcja_Plon (
    id_transakcji INT REFERENCES Transakcja(id_transakcji) ON DELETE CASCADE,
    id_plonu INT REFERENCES Plon(id_plonu) ON DELETE CASCADE,
    ilosc DECIMAL(10,2) NOT NULL CHECK (ilosc > 0),
    cena_jednostkowa DECIMAL(12,2) NOT NULL CHECK (cena_jednostkowa >= 0),

    PRIMARY KEY (id_transakcji, id_plonu)
);

-- ===============================
ALTER TABLE pole
ADD CONSTRAINT unikalna_nazwa_pola_rolnika UNIQUE (id_rolnika, nazwa);






CREATE OR REPLACE VIEW lista_uzytkownikow AS
SELECT 
    u.id_uzytkownika,
    u.login,
    u.rola,
    u.aktywny,
    p.imie,
    p.nazwisko,
    p.id_rolnika AS przypisany_rolnik
FROM Uzytkownik u
LEFT JOIN Pracownik p
    ON u.id_uzytkownika = p.id_uzytkownika
ORDER BY u.id_uzytkownika;


CREATE OR REPLACE VIEW raport_statystyki_dod AS
(
    -- 1. Pracownicy powyżej średniej liczby zadań (aktualny rok)
    SELECT
        'pracownik_powyzej_sredniej' AS typ,
        p.imie || ' ' || p.nazwisko AS nazwa,
        COUNT(*) AS wartosc,
        p.id_rolnika
    FROM Zadanie z
    JOIN Pracownik p ON z.id_pracownika = p.id_pracownika
    WHERE EXTRACT(YEAR FROM z.data_start) = EXTRACT(YEAR FROM CURRENT_DATE)
    GROUP BY p.imie, p.nazwisko, p.id_rolnika
    HAVING COUNT(*) > (
        SELECT AVG(cnt)
        FROM (
            SELECT COUNT(*) AS cnt
            FROM Zadanie
            WHERE EXTRACT(YEAR FROM data_start) = EXTRACT(YEAR FROM CURRENT_DATE)
              AND id_rolnika = p.id_rolnika
            GROUP BY id_pracownika
        ) t
    )
)
UNION ALL
(
    -- 2. Maszyny używane rzadziej niż średnia
    SELECT
        'rzadko_uzywana_maszyna' AS typ,
        m.nazwa AS nazwa,
        COUNT(*) AS wartosc,
        m.id_rolnika
    FROM Zadanie z
    JOIN Maszyna m ON z.id_maszyny = m.id_maszyny
    GROUP BY m.nazwa, m.id_rolnika
    HAVING COUNT(*) < (
        SELECT AVG(cnt)
        FROM (
            SELECT COUNT(*) AS cnt
            FROM Zadanie
            WHERE id_rolnika = m.id_rolnika
            GROUP BY id_maszyny
        ) t
    )
)
UNION ALL
(
    -- 3. Pola z więcej niż 3 uprawami
    SELECT
        'pole_z_duza_liczba_upraw' AS typ,
        p.nazwa AS nazwa,
        COUNT(*) AS wartosc,
        p.id_rolnika
    FROM Uprawa u
    JOIN Pole p ON u.id_pola = p.id_pola
    GROUP BY p.nazwa, p.id_rolnika
    HAVING COUNT(*) > 3
)
UNION ALL
(
    -- 4. Rośliny uprawiane częściej niż średnia
    SELECT
        'popularna_roslina' AS typ,
        u.nazwa AS nazwa,
        COUNT(*) AS wartosc,
        p.id_rolnika
    FROM Uprawa u
    JOIN Pole p ON u.id_pola = p.id_pola
    GROUP BY u.nazwa, p.id_rolnika
    HAVING COUNT(*) > (
        SELECT AVG(cnt)
        FROM (
            SELECT COUNT(*) AS cnt
            FROM Uprawa u2
            JOIN Pole p2 ON u2.id_pola = p2.id_pola
            WHERE p2.id_rolnika = p.id_rolnika
            GROUP BY u2.nazwa
        ) t
    )
)
UNION ALL
(
    -- 5. Dni z większą liczbą zadań niż średnia
    SELECT
        'rekordowy_dzien' AS typ,
        TO_CHAR(z.data_start, 'YYYY-MM-DD') AS nazwa,
        COUNT(*) AS wartosc,
        z.id_rolnika
    FROM Zadanie z
    GROUP BY z.data_start, z.id_rolnika
    HAVING COUNT(*) > (
        SELECT AVG(cnt)
        FROM (
            SELECT COUNT(*) AS cnt
            FROM Zadanie
            WHERE id_rolnika = z.id_rolnika
            GROUP BY data_start
        ) t
    )
);



CREATE OR REPLACE VIEW raport_statystyki_glowne AS
SELECT
    r.id_uzytkownika AS id_rolnika,

    -- 1. Najbardziej dochodowy miesiąc (aktualny rok)
    (
        SELECT TO_CHAR(DATE_TRUNC('month', t.data_transakcji), 'YYYY-MM')
        FROM Transakcja t
        WHERE t.id_rolnika = r.id_uzytkownika
          AND EXTRACT(YEAR FROM t.data_transakcji) = EXTRACT(YEAR FROM CURRENT_DATE)
        GROUP BY DATE_TRUNC('month', t.data_transakcji)
        ORDER BY SUM(CASE WHEN t.typ='sprzedaz' THEN t.kwota ELSE -t.kwota END) DESC
        LIMIT 1
    ) AS najbardziej_dochodowy_miesiac,

    -- 2. Największa jednorazowa transakcja
    (
        SELECT MAX(kwota)
        FROM Transakcja t
        WHERE t.id_rolnika = r.id_uzytkownika
    ) AS najwieksza_transakcja,

    -- 3. Średnia wartość transakcji
    (
        SELECT AVG(kwota)
        FROM Transakcja t
        WHERE t.id_rolnika = r.id_uzytkownika
    ) AS srednia_wartosc_transakcji,

    -- 4. Łączny zysk netto w aktualnym roku
    (
        SELECT SUM(CASE WHEN t.typ='sprzedaz' THEN t.kwota ELSE -t.kwota END)
        FROM Transakcja t
        WHERE t.id_rolnika = r.id_uzytkownika
          AND EXTRACT(YEAR FROM t.data_transakcji) = EXTRACT(YEAR FROM CURRENT_DATE)
    ) AS zysk_netto_rok,

    -- 5. Najczęściej używana maszyna
    (
        SELECT m.nazwa
        FROM Zadanie z
        JOIN Maszyna m ON z.id_maszyny = m.id_maszyny
        WHERE z.id_rolnika = r.id_uzytkownika
        GROUP BY m.nazwa
        ORDER BY COUNT(*) DESC
        LIMIT 1
    ) AS najczesciej_uzywana_maszyna,

    -- 6. Pracownik z największą liczbą wykonanych zadań
    (
        SELECT p.imie || ' ' || p.nazwisko
        FROM Zadanie z
        JOIN Pracownik p ON z.id_pracownika = p.id_pracownika
        WHERE z.id_rolnika = r.id_uzytkownika
        GROUP BY p.imie, p.nazwisko
        ORDER BY COUNT(*) DESC
        LIMIT 1
    ) AS najbardziej_aktywny_pracownik,

    -- 7. Średnia liczba zadań na pracownika
    (
        SELECT AVG(cnt)
        FROM (
            SELECT COUNT(*) AS cnt
            FROM Zadanie z
            WHERE z.id_rolnika = r.id_uzytkownika
            GROUP BY z.id_pracownika
        ) t
    ) AS srednia_zadan_na_pracownika,

    -- 8. Łączna liczba zwierząt
    (
        SELECT SUM(ilosc)
        FROM Zwierze z
        WHERE z.id_rolnika = r.id_uzytkownika
    ) AS laczna_liczba_zwierzat,

    -- 9. Najczęściej uprawiana roślina
    (
        SELECT u.nazwa
        FROM Uprawa u
        JOIN Pole p ON u.id_pola = p.id_pola
        WHERE p.id_rolnika = r.id_uzytkownika
        GROUP BY u.nazwa
        ORDER BY COUNT(*) DESC
        LIMIT 1
    ) AS najczesciej_uprawiana_roslina,

    -- 10. Pole z największą liczbą upraw
    (
        SELECT p.nazwa
        FROM Uprawa u
        JOIN Pole p ON u.id_pola = p.id_pola
        WHERE p.id_rolnika = r.id_uzytkownika
        GROUP BY p.nazwa
        ORDER BY COUNT(*) DESC
        LIMIT 1
    ) AS pole_z_najwieksza_liczba_upraw

FROM Uzytkownik r
WHERE r.rola = 'rolnik';



CREATE OR REPLACE VIEW view_budynki_info AS
SELECT
    b.id_budynku,
    b.nazwa,
    b.typ,
    b.pojemnosc,
    b.id_rolnika,

    -- liczba zwierząt w budynku
    (
        SELECT COALESCE(SUM(z.ilosc), 0)
        FROM Zwierze z
        WHERE z.id_budynku = b.id_budynku
    ) AS liczba_zwierzat,

    -- liczba maszyn w budynku
    (
        SELECT COUNT(*)
        FROM Maszyna m
        WHERE m.id_budynku = b.id_budynku
    ) AS liczba_maszyn

FROM Budynek b;




CREATE OR REPLACE VIEW view_dashboard_pracownicy AS
SELECT
    p.id_pracownika,
    p.imie,
    p.nazwisko,
    p.stanowisko,
    p.telefon,
    p.data_zatrudnienia,
    p.id_rolnika,

    -- zadania aktywne
    COUNT(z.id_zadania) FILTER (
        WHERE z.status IN ('planowane', 'w_trakcie')
    ) AS zadania_aktywne,

    -- zadania wykonane
    COUNT(z.id_zadania) FILTER (
        WHERE z.status = 'wykonane'
    ) AS zadania_wykonane

FROM Pracownik p
LEFT JOIN Zadanie z
    ON z.id_pracownika = p.id_pracownika

GROUP BY
    p.id_pracownika,
    p.imie,
    p.nazwisko,
    p.stanowisko,
    p.telefon,
    p.data_zatrudnienia,
    p.id_rolnika;



CREATE OR REPLACE VIEW view_maszyny_info AS
SELECT
    m.id_maszyny,
    m.nazwa,
    m.typ,
    m.data_przegladu,
    m.id_rolnika,

    b.nazwa AS nazwa_budynku,

    -- Liczba aktywnych zadań korzystających z maszyny
    (
        SELECT COUNT(*)
        FROM Zadanie z
        WHERE z.id_maszyny = m.id_maszyny
          AND z.status NOT IN ('wykonane', 'anulowane')
    ) AS aktywne_zadania

FROM Maszyna m
LEFT JOIN Budynek b 
       ON b.id_budynku = m.id_budynku;



CREATE OR REPLACE VIEW view_plony_info
AS SELECT p.id_plonu,
    p.nazwa,
    m.nazwa AS magazyn,
    p.ilosc,
    z.gatunek AS gatunek_zwierzecia,
    z.rasa AS rasa_zwierzecia,
    u.nazwa AS nazwa_uprawy,
    p.data_pozyskania,
    m.id_rolnika
   FROM gospodarka_testy.plon p
     JOIN gospodarka_testy.magazyn m ON m.id_magazynu = p.id_magazynu
     LEFT JOIN gospodarka_testy.zwierze z ON z.id_zwierzecia = p.id_zwierzecia
     LEFT JOIN gospodarka_testy.uprawa u ON u.id_uprawy = p.id_uprawy;



CREATE OR REPLACE VIEW view_pole_dashboard AS
SELECT 
    p.id_pola,
    p.id_rolnika,
    p.nazwa,
    p.powierzchnia,
    COUNT(u.id_uprawy) AS liczba_aktywnych_upraw
FROM pole p
LEFT JOIN uprawa u
    ON p.id_pola = u.id_pola
   AND u.data_zbioru IS NULL     -- tylko uprawy niezebrane
GROUP BY 
    p.id_pola, p.id_rolnika, p.nazwa, p.powierzchnia
ORDER BY p.id_pola;


CREATE OR REPLACE VIEW view_pracownik_zadania AS
SELECT 
    z.id_zadania,
    z.nazwa,
    z.status,
    z.data_start,
    z.data_koniec,
    p.id_pracownika,
    p.imie,
    p.nazwisko
FROM Zadanie z
LEFT JOIN Pracownik p ON p.id_pracownika = z.id_pracownika;


CREATE OR REPLACE VIEW view_przeglady_alert AS
SELECT 
    m.id_maszyny,
    m.nazwa,
    m.typ,
    m.data_przegladu,
    m.id_rolnika,
    (m.data_przegladu - CURRENT_DATE) AS dni_do_przegladu
FROM Maszyna m
WHERE 
    m.data_przegladu IS NOT NULL
    AND m.data_przegladu <= CURRENT_DATE + INTERVAL '3 days'
    AND m.data_przegladu >= CURRENT_DATE;



CREATE OR REPLACE VIEW view_surowce_info AS
SELECT 
    s.id_surowca,
    s.nazwa,
    s.kategoria,
    s.ilosc,
    m.nazwa AS magazyn,
    m.id_rolnika
FROM Surowiec s
JOIN Magazyn m ON m.id_magazynu = s.id_magazynu;




CREATE OR REPLACE VIEW view_uprawy_info AS
SELECT 
    u.id_uprawy,
    p.nazwa AS nazwa_pola,
    u.nazwa AS nazwa_uprawy,
    u.data_siewu,
    u.data_zbioru,
    p.id_rolnika
FROM uprawa u
JOIN pole p ON u.id_pola = p.id_pola
ORDER BY u.id_uprawy;



CREATE OR REPLACE VIEW view_zadania_szczegoly AS
SELECT
    z.id_zadania,
    z.nazwa AS zadanie_nazwa,
    z.opis,
    z.status,
    z.data_start,
    z.data_koniec,
    z.id_pola,
    p.nazwa AS pole_nazwa,
    z.id_uprawy,
    u.nazwa AS uprawa_nazwa,
    z.id_pracownika,
    pr.imie AS pracownik_imie,
    pr.nazwisko AS pracownik_nazwisko,
    z.id_maszyny,
    m.nazwa AS maszyna_nazwa,
    z.id_budynku,
    b.nazwa AS budynek_nazwa,
    z.id_zwierzecia,
    zw.gatunek AS zwierze_gatunek,
    z.id_rolnika,
    rol.login AS rolnik_login
FROM Zadanie z
LEFT JOIN Pole p ON z.id_pola = p.id_pola
LEFT JOIN Uprawa u ON z.id_uprawy = u.id_uprawy
LEFT JOIN Pracownik pr ON z.id_pracownika = pr.id_pracownika
LEFT JOIN Maszyna m ON z.id_maszyny = m.id_maszyny
LEFT JOIN Budynek b ON z.id_budynku = b.id_budynku
LEFT JOIN Zwierze zw ON z.id_zwierzecia = zw.id_zwierzecia
LEFT JOIN Uzytkownik rol ON z.id_rolnika = rol.id_uzytkownika;




CREATE OR REPLACE VIEW view_zawartosc_magazynow AS
SELECT 
    m.id_magazynu,
    m.nazwa AS nazwa_magazynu,
    'plon' AS typ,
    p.id_plonu AS id_elementu,
    p.nazwa AS nazwa,
    p.ilosc
FROM Magazyn m
LEFT JOIN Plon p ON p.id_magazynu = m.id_magazynu

UNION ALL

SELECT
    m.id_magazynu,
    m.nazwa AS nazwa_magazynu,
    'surowiec' AS typ,
    s.id_surowca AS id_elementu,
    s.nazwa AS nazwa,
    s.ilosc
FROM Magazyn m
LEFT JOIN Surowiec s ON s.id_magazynu = m.id_magazynu;




CREATE OR REPLACE VIEW view_zwierzeta_info AS
SELECT
    z.id_zwierzecia,
    z.gatunek,
    z.rasa,
    z.ilosc,
    z.id_rolnika,
    b.nazwa AS budynek
FROM Zwierze z
LEFT JOIN Budynek b ON b.id_budynku = z.id_budynku;



-- POLE --------------------------
CREATE OR REPLACE FUNCTION trg_unique_nazwa_pola()
RETURNS TRIGGER AS $$
BEGIN
    IF EXISTS (
        SELECT 1
        FROM Pole
        WHERE id_rolnika = NEW.id_rolnika
          AND LOWER(nazwa) = LOWER(NEW.nazwa)
          AND (TG_OP = 'UPDATE' AND id_pola <> NEW.id_pola OR TG_OP = 'INSERT')
    ) THEN
        RAISE EXCEPTION 'Rolnik % już ma pole o nazwie "%"', NEW.id_rolnika, NEW.nazwa;
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;


CREATE TRIGGER check_unique_pole_name
BEFORE INSERT OR UPDATE ON Pole
FOR EACH ROW
EXECUTE FUNCTION trg_unique_nazwa_pola();


-- Budynek ----------------------------
CREATE OR REPLACE FUNCTION trg_unique_budynek()
RETURNS TRIGGER AS $$
BEGIN
    IF EXISTS (
        SELECT 1 FROM budynek
        WHERE id_rolnika = NEW.id_rolnika
          AND LOWER(nazwa) = LOWER(NEW.nazwa)
          AND (TG_OP = 'INSERT' OR id_budynku <> NEW.id_budynku)
    ) THEN
        RAISE EXCEPTION 'Rolnik % już ma budynek o nazwie "%"', 
            NEW.id_rolnika, NEW.nazwa;
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER check_unique_budynek
BEFORE INSERT OR UPDATE ON budynek
FOR EACH ROW
EXECUTE FUNCTION trg_unique_budynek();

-- Maszyna ------------------------------
CREATE OR REPLACE FUNCTION trg_unique_maszyna()
RETURNS TRIGGER AS $$
BEGIN
    IF EXISTS (
        SELECT 1 FROM maszyna
        WHERE id_rolnika = NEW.id_rolnika
          AND LOWER(nazwa) = LOWER(NEW.nazwa)
          AND (TG_OP = 'INSERT' OR id_maszyny <> NEW.id_maszyny)
    ) THEN
        RAISE EXCEPTION 'Rolnik % już ma maszynę o nazwie "%"', 
            NEW.id_rolnika, NEW.nazwa;
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER check_unique_maszyna
BEFORE INSERT OR UPDATE ON maszyna
FOR EACH ROW
EXECUTE FUNCTION trg_unique_maszyna();




-- Pracownik ---------------------------

CREATE OR REPLACE FUNCTION trg_unique_pracownik()
RETURNS TRIGGER AS $$
BEGIN
    IF EXISTS (
        SELECT 1 
        FROM pracownik
        WHERE id_rolnika = NEW.id_rolnika
          AND LOWER(imie) = LOWER(NEW.imie)
          AND LOWER(nazwisko) = LOWER(NEW.nazwisko)
          AND (TG_OP = 'INSERT' OR id_pracownika <> NEW.id_pracownika)
    ) THEN
        RAISE EXCEPTION 'Pracownik % % już istnieje u rolnika %',
            NEW.imie, NEW.nazwisko, NEW.id_rolnika;
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER check_unique_pracownik
BEFORE INSERT OR UPDATE ON pracownik
FOR EACH ROW
EXECUTE FUNCTION trg_unique_pracownik();




-- Zwierze --------------------------

CREATE OR REPLACE FUNCTION trg_unique_zwierze()
RETURNS TRIGGER AS $$
BEGIN
    IF EXISTS (
        SELECT 1 
        FROM zwierze
        WHERE id_rolnika = NEW.id_rolnika
          AND LOWER(gatunek) = LOWER(NEW.gatunek)
          AND LOWER(rasa) = LOWER(NEW.rasa)
          AND (TG_OP = 'INSERT' OR id_zwierzecia <> NEW.id_zwierzecia)
    ) THEN
        RAISE EXCEPTION 'Rolnik % już ma zwierzę: % (%).',
            NEW.id_rolnika, NEW.gatunek, NEW.rasa;
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER check_unique_zwierze
BEFORE INSERT OR UPDATE ON zwierze
FOR EACH ROW
EXECUTE FUNCTION trg_unique_zwierze();


-- Surowiec -----------------------

CREATE OR REPLACE FUNCTION trg_unique_surowiec()
RETURNS TRIGGER AS $$
BEGIN
    IF EXISTS (
        SELECT 1 
        FROM Surowiec
        WHERE id_magazynu = NEW.id_magazynu
          AND LOWER(nazwa) = LOWER(NEW.nazwa)
          AND (TG_OP = 'INSERT' OR id_surowca <> NEW.id_surowca)
    ) THEN
        RAISE EXCEPTION 'Magazyn % już zawiera surowiec o nazwie "%"',
            NEW.id_magazynu, NEW.nazwa;
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER check_unique_surowiec
BEFORE INSERT OR UPDATE ON Surowiec
FOR EACH ROW
EXECUTE FUNCTION trg_unique_surowiec();


-- Plon ---------------------------

CREATE OR REPLACE FUNCTION trg_unique_plon()
RETURNS TRIGGER AS $$
BEGIN
    IF EXISTS (
        SELECT 1 
        FROM plon
        WHERE id_magazynu = NEW.id_magazynu
          AND LOWER(nazwa) = LOWER(NEW.nazwa)
          AND (TG_OP = 'INSERT' OR id_plonu <> NEW.id_plonu)
    ) THEN
        RAISE EXCEPTION 'Magazyn % już ma plon o nazwie "%"',
            NEW.id_magazynu, NEW.nazwa;
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER check_unique_plon
BEFORE INSERT OR UPDATE ON plon
FOR EACH ROW
EXECUTE FUNCTION trg_unique_plon();

-- Uprawa -----------------------------

CREATE OR REPLACE FUNCTION trg_unique_uprawa()
RETURNS TRIGGER AS $$
BEGIN
    IF EXISTS (
        SELECT 1 
        FROM Uprawa
        WHERE id_pola = NEW.id_pola
          AND LOWER(nazwa) = LOWER(NEW.nazwa)
          AND (data_zbioru IS NULL OR data_zbioru > CURRENT_DATE)
          AND (TG_OP = 'INSERT' OR id_uprawy <> NEW.id_uprawy)
    ) THEN
        RAISE EXCEPTION 
            'Na polu % istnieje aktywna uprawa o nazwie "%" (niezebrana).',
            NEW.id_pola, NEW.nazwa;
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER check_unique_uprawa
BEFORE INSERT OR UPDATE ON Uprawa
FOR EACH ROW
EXECUTE FUNCTION trg_unique_uprawa();


-- Magazyn -------------------------------

CREATE OR REPLACE FUNCTION trg_unique_magazyn()
RETURNS TRIGGER AS $$
BEGIN
    IF EXISTS (
        SELECT 1 FROM Magazyn
        WHERE id_rolnika = NEW.id_rolnika
          AND LOWER(nazwa) = LOWER(NEW.nazwa)
          AND id_magazynu <> NEW.id_magazynu
    ) THEN
        RAISE EXCEPTION 'Rolnik ma już magazyn o nazwie %', NEW.nazwa;
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER check_unique_magazyn
BEFORE INSERT OR UPDATE ON Magazyn
FOR EACH ROW
EXECUTE FUNCTION trg_unique_magazyn();


----------- ZWIERZĄTA ----------

CREATE OR REPLACE FUNCTION dodaj_zwierze(
    p_id_rolnika INT,
    p_id_budynku INT,
    p_gatunek VARCHAR,
    p_rasa VARCHAR,
    p_ilosc INT
)
RETURNS INT AS $$
DECLARE
    new_id INT;
    owner_id INT;
BEGIN
    -- Walidacja: budynek należy do rolnika
    SELECT id_rolnika INTO owner_id
    FROM budynek
    WHERE id_budynku = p_id_budynku;

    IF owner_id IS NULL THEN
        RAISE EXCEPTION 'Budynek o ID % nie istnieje', p_id_budynku;
    END IF;

    IF owner_id <> p_id_rolnika THEN
        RAISE EXCEPTION 'Budynek nie należy do rolnika %', p_id_rolnika;
    END IF;

    -- Wstawienie zwierzęcia
    INSERT INTO zwierze(id_rolnika, id_budynku, gatunek, rasa, ilosc)
    VALUES ( p_id_rolnika, p_id_budynku, p_gatunek, p_rasa, p_ilosc)
    RETURNING id_zwierzecia INTO new_id;

    RETURN new_id;
END;
$$ LANGUAGE plpgsql;



CREATE OR REPLACE FUNCTION edytuj_zwierze(
    p_id_zwierzecia INT,
    p_nowy_budynek INT DEFAULT NULL,
    p_nowa_ilosc INT DEFAULT NULL
)
RETURNS VOID AS $$
DECLARE
    stary_rolnik INT;
    owner INT;
BEGIN
    -- Pobieramy właściciela zwierzęcia
    SELECT id_rolnika INTO stary_rolnik
    FROM zwierze
    WHERE id_zwierzecia = p_id_zwierzecia;

    IF stary_rolnik IS NULL THEN
        RAISE EXCEPTION 'Zwierzę o ID % nie istnieje', p_id_zwierzecia;
    END IF;

    -- Jeśli zmieniamy budynek → walidacja własności
    IF p_nowy_budynek IS NOT NULL THEN
        SELECT id_rolnika INTO owner
        FROM budynek
        WHERE id_budynku = p_nowy_budynek;

        IF owner IS NULL THEN
            RAISE EXCEPTION 'Budynek o ID % nie istnieje', p_nowy_budynek;
        END IF;

        IF owner <> stary_rolnik THEN
            RAISE EXCEPTION 'Nie możesz przenieść zwierzęcia do budynku nienależącego do rolnika %', stary_rolnik;
        END IF;
    END IF;

    -- Aktualizacja
    UPDATE zwierze
    SET id_budynku = COALESCE( p_nowy_budynek, id_budynku),
        ilosc      = COALESCE( p_nowa_ilosc, ilosc)
    WHERE id_zwierzecia = p_id_zwierzecia;
END;
$$ LANGUAGE plpgsql;



CREATE OR REPLACE FUNCTION usun_zwierze(
	p_id_zwierzecia INT
)
RETURNS VOID AS $$
DECLARE
    blokada INT;
BEGIN
    -- Sprawdzamy czy są powiązane aktywne zadania
    SELECT COUNT(*) INTO blokada
    FROM zadanie
    WHERE id_zwierzecia = p_id_zwierzecia
      AND status NOT IN ('wykonane','anulowane');

    IF blokada > 0 THEN
        RAISE EXCEPTION
            'Nie można usunąć zwierzęcia – istnieją powiązane zadania w toku.';
    END IF;

    -- Usuwanie
    DELETE FROM zwierze
    WHERE id_zwierzecia = p_id_zwierzecia;

    IF NOT FOUND THEN
        RAISE EXCEPTION 'Zwierzę o ID % nie istnieje', p_id_zwierzecia;
    END IF;
END;
$$ LANGUAGE plpgsql;


--------------- UPRAWY ----------------

CREATE OR REPLACE FUNCTION dodaj_uprawe(
    p_id_rolnika INT,
    p_id_pola INT,
    p_nazwa VARCHAR
)
RETURNS INT AS $$
DECLARE
    v_exists BOOLEAN;
    v_id_uprawy INT;
BEGIN
    -- Pole musi należeć do rolnika
    SELECT TRUE INTO v_exists
    FROM Pole
    WHERE id_pola = p_id_pola
      AND id_rolnika = p_id_rolnika;

    IF NOT v_exists THEN
        RAISE EXCEPTION 'Pole % nie należy do rolnika %', p_id_pola, p_id_rolnika;
    END IF;

    -- Dodanie uprawy
    INSERT INTO Uprawa(id_pola, nazwa, data_siewu, data_zbioru)
    VALUES (p_id_pola, p_nazwa, NULL, NULL)
    RETURNING id_uprawy INTO v_id_uprawy;

    RETURN v_id_uprawy;
END;
$$ LANGUAGE plpgsql;




CREATE OR REPLACE FUNCTION edytuj_uprawe(
    p_id_rolnika INT,
    p_id_uprawy INT,
    p_nazwa VARCHAR DEFAULT NULL,
    p_data_siewu DATE DEFAULT NULL,
    p_data_zbioru DATE DEFAULT NULL
)
RETURNS VOID AS $$
DECLARE
    v_id_pola INT;
BEGIN
    -- Pobranie pola dla uprawy
    SELECT u.id_pola INTO v_id_pola
    FROM Uprawa u
    JOIN Pole p ON p.id_pola = u.id_pola
    WHERE id_uprawy = p_id_uprawy
      AND p.id_rolnika = p_id_rolnika;

    IF v_id_pola IS NULL THEN
        RAISE EXCEPTION 'Uprawa % nie należy do rolnika %', 
            p_id_uprawy, p_id_rolnika;
    END IF;

    -- Walidacja dat
    IF p_data_siewu IS NOT NULL AND p_data_zbioru IS NOT NULL THEN
        IF p_data_zbioru < p_data_siewu THEN
            RAISE EXCEPTION 'Data zbioru nie może być wcześniejsza niż data siewu.';
        END IF;
    END IF;

    -- Aktualizacja uprawy
    UPDATE Uprawa
    SET nazwa = COALESCE(p_nazwa, nazwa),
        data_siewu = COALESCE(p_data_siewu, data_siewu),
        data_zbioru = COALESCE(p_data_zbioru, data_zbioru)
    WHERE id_uprawy = p_id_uprawy;
END;
$$ LANGUAGE plpgsql;







CREATE OR REPLACE FUNCTION usun_uprawe(
    p_id_rolnika INT,
    p_id_uprawy INT
)
RETURNS VOID AS $$
DECLARE
    v_exists BOOLEAN;
BEGIN
    -- Sprawdzenie, czy uprawa należy do rolnika
    SELECT TRUE INTO v_exists
    FROM Uprawa u
    JOIN Pole p ON p.id_pola = u.id_pola
    WHERE u.id_uprawy = p_id_uprawy
      AND p.id_rolnika = p_id_rolnika;

    IF NOT v_exists THEN
        RAISE EXCEPTION 'Rolnik % nie ma prawa usunąć uprawy %', 
            p_id_rolnika, p_id_uprawy;
    END IF;


    -- Usunięcie
    DELETE FROM Uprawa
    WHERE id_uprawy = p_id_uprawy;
END;
$$ LANGUAGE plpgsql;


------------ SUROWCE -------------

CREATE OR REPLACE FUNCTION dodaj_surowiec(
    p_id_rolnika INT,
    p_id_magazynu INT,
    p_nazwa VARCHAR,
    p_kategoria VARCHAR,
    p_ilosc DECIMAL DEFAULT 0
)
RETURNS INT
LANGUAGE plpgsql
AS $$
DECLARE
    v_new_id INT;
    v_m_owner INT;
    v_exists BOOLEAN;
BEGIN
    -- 1. Czy magazyn należy do rolnika?
    SELECT id_rolnika INTO v_m_owner
    FROM Magazyn
    WHERE id_magazynu = p_id_magazynu;

    IF v_m_owner IS NULL OR v_m_owner <> p_id_rolnika THEN
        RAISE EXCEPTION 'Magazyn % nie należy do rolnika %', p_id_magazynu, p_id_rolnika;
    END IF;

    -- 2. Walidacja ilości
    IF p_ilosc < 0 THEN
        RAISE EXCEPTION 'Ilość surowca nie może być ujemna.';
    END IF;


    -- 3. Dodanie surowca
    INSERT INTO Surowiec(id_magazynu, nazwa, ilosc, kategoria)
    VALUES (p_id_magazynu, p_nazwa, p_ilosc, p_kategoria)
    RETURNING id_surowca INTO v_new_id;

    RETURN v_new_id;
END;
$$;

CREATE OR REPLACE FUNCTION edytuj_surowiec(
    p_id_rolnika INT,
    p_id_surowca INT,
    p_nazwa VARCHAR,
    p_kategoria VARCHAR,
    p_ilosc DECIMAL,
    p_id_magazynu INT        
)
RETURNS VOID
LANGUAGE plpgsql
AS $$
DECLARE
    v_old_magazyn INT;
    v_new_mag_owner INT;
BEGIN
    -----------------------------------------------------------
    -- 1. Pobranie starego magazynu i weryfikacja właściciela
    -----------------------------------------------------------
    SELECT s.id_magazynu
    INTO v_old_magazyn
    FROM Surowiec s
    JOIN Magazyn m ON m.id_magazynu = s.id_magazynu
    WHERE s.id_surowca = p_id_surowca
      AND m.id_rolnika = p_id_rolnika;

    IF v_old_magazyn IS NULL THEN
        RAISE EXCEPTION 'Surowiec % nie należy do rolnika %',
            p_id_surowca, p_id_rolnika;
    END IF;

    -----------------------------------------------------------
    -- 2. Walidacja ilości
    -----------------------------------------------------------
    IF p_ilosc < 0 THEN
        RAISE EXCEPTION 'Ilość surowca nie może być ujemna.';
    END IF;

    -----------------------------------------------------------
    -- 3. Jeśli zmieniamy magazyn -> sprawdź właściciela
    -----------------------------------------------------------
    IF p_id_magazynu IS NOT NULL AND p_id_magazynu <> v_old_magazyn THEN
        
        SELECT id_rolnika INTO v_new_mag_owner
        FROM Magazyn
        WHERE id_magazynu = p_id_magazynu;

        IF v_new_mag_owner IS NULL OR v_new_mag_owner <> p_id_rolnika THEN
            RAISE EXCEPTION 'Magazyn % nie należy do rolnika %',
                  p_id_magazynu, p_id_rolnika;
        END IF;

        -----------------------------------------------------------
        -- 4. Jeśli zmieniamy magazyn -> sprawdzić unikalność nazwy
        -----------------------------------------------------------
        IF p_nazwa IS NOT NULL THEN
            IF EXISTS (
                SELECT 1 FROM Surowiec
                WHERE id_magazynu = p_id_magazynu
                  AND LOWER(nazwa) = LOWER(p_nazwa)
            ) THEN
                RAISE EXCEPTION 
                    'W magazynie % istnieje już surowiec o nazwie "%"', 
                    p_id_magazynu, p_nazwa;
            END IF;
        ELSE
            -- jeśli nie zmieniamy nazwy -> sprawdzamy starą
            IF EXISTS (
                SELECT 1 FROM Surowiec
                WHERE id_magazynu = p_id_magazynu
                  AND LOWER(nazwa) = LOWER((SELECT nazwa FROM Surowiec WHERE id_surowca = p_id_surowca))
            ) THEN
                RAISE EXCEPTION 
                    'W magazynie % istnieje już surowiec o tej samej nazwie', 
                    p_id_magazynu;
            END IF;
        END IF;

    END IF;

    -----------------------------------------------------------
    -- 5. Aktualizacja danych
    -----------------------------------------------------------
    UPDATE Surowiec
    SET 
        nazwa = COALESCE(p_nazwa, nazwa),
        kategoria = COALESCE(p_kategoria, kategoria),
        ilosc = p_ilosc,
        id_magazynu = COALESCE(p_id_magazynu, id_magazynu)
    WHERE id_surowca = p_id_surowca;

END;
$$;



CREATE OR REPLACE FUNCTION usun_surowiec(
    p_id_rolnika INT,
    p_id_surowca INT
)
RETURNS VOID
LANGUAGE plpgsql
AS $$
DECLARE
    v_ilosc DECIMAL;
    v_magazyn INT;
BEGIN
    -- Pobranie magazynu i ilości
    SELECT s.ilosc, s.id_magazynu
    INTO v_ilosc, v_magazyn
    FROM Surowiec s
    JOIN Magazyn m ON m.id_magazynu = s.id_magazynu
    WHERE s.id_surowca = p_id_surowca
      AND m.id_rolnika = p_id_rolnika;

    IF v_magazyn IS NULL THEN
        RAISE EXCEPTION 'Surowiec % nie należy do rolnika %', p_id_surowca, p_id_rolnika;
    END IF;

	IF v_ilosc <> 0 THEN 
		RAISE EXCEPTION 'By usunąć rekord surowca, musi być go 0 w magazynie';
	END IF;

    DELETE FROM Surowiec
    WHERE id_surowca = p_id_surowca;
END;
$$;



------------ PLONY -------------


CREATE OR REPLACE FUNCTION dodaj_plon(
    p_rolnik_id INT,
    p_id_magazynu INT,
    p_nazwa VARCHAR(50),
    p_ilosc DECIMAL(10,2),
    p_id_uprawy INT DEFAULT NULL,
    p_id_zwierzecia INT DEFAULT NULL,
    p_data_pozyskania DATE DEFAULT NULL
)
RETURNS INT
LANGUAGE plpgsql
AS $$
DECLARE
    new_id INT;
BEGIN
    IF p_ilosc < 0 THEN
        RAISE EXCEPTION 'Ilość plonu nie może być ujemna.';
    END IF;

    -- Walidacja magazynu
    IF NOT EXISTS (
        SELECT 1 FROM Magazyn 
        WHERE id_magazynu = p_id_magazynu 
          AND id_rolnika = p_rolnik_id
    ) THEN
        RAISE EXCEPTION 
            'Magazyn % nie należy do rolnika %', 
            p_id_magazynu, p_rolnik_id;
    END IF;

    INSERT INTO Plon(id_magazynu, id_uprawy, id_zwierzecia, 
                     nazwa, ilosc, data_pozyskania)
    VALUES (
        p_id_magazynu,
        p_id_uprawy,
        p_id_zwierzecia,
        p_nazwa,
        p_ilosc,
        NULL
    )
    RETURNING id_plonu INTO new_id;

    RETURN new_id;
END;
$$;

CREATE OR REPLACE FUNCTION edytuj_plon(
    p_rolnik_id INT,
    p_id_plonu INT,
    p_id_magazynu INT,
    p_nazwa VARCHAR(50),
    p_ilosc DECIMAL(10,2),
    p_data_pozyskania DATE DEFAULT NULL
)
RETURNS VOID
LANGUAGE plpgsql
AS $$
BEGIN
    IF p_ilosc < 0 THEN
        RAISE EXCEPTION 'Ilość plonu nie może być ujemna.';
    END IF;

    -- Sprawdzenie czy plon należy do rolnika
    IF NOT EXISTS (
        SELECT 1
        FROM Plon p
        JOIN Magazyn m ON m.id_magazynu = p.id_magazynu
        WHERE p.id_plonu = p_id_plonu
          AND m.id_rolnika = p_rolnik_id
    ) THEN
        RAISE EXCEPTION 
            'Plon % nie należy do rolnika %', 
            p_id_plonu, p_rolnik_id;
    END IF;

    -- Walidacja nowego magazynu
    IF NOT EXISTS (
        SELECT 1 
        FROM Magazyn
        WHERE id_magazynu = p_id_magazynu
          AND id_rolnika = p_rolnik_id
    ) THEN
        RAISE EXCEPTION 
            'Magazyn % nie należy do rolnika %', 
            p_id_magazynu, p_rolnik_id;
    END IF;

    UPDATE Plon
    SET id_magazynu = p_id_magazynu,
        nazwa = p_nazwa,
        ilosc = p_ilosc,
        data_pozyskania = COALESCE(p_data_pozyskania, data_pozyskania)
    WHERE id_plonu = p_id_plonu;
END;
$$;


CREATE OR REPLACE FUNCTION usun_plon(
    p_rolnik_id INT,
    p_id_plonu INT
)
RETURNS VOID
LANGUAGE plpgsql
AS $$
BEGIN
    -- Walidacja właściciela
    IF NOT EXISTS (
        SELECT 1
        FROM Plon p
        JOIN Magazyn m ON m.id_magazynu = p.id_magazynu
        WHERE p.id_plonu = p_id_plonu
          AND m.id_rolnika = p_rolnik_id
    ) THEN
        RAISE EXCEPTION 
            'Plon % nie należy do rolnika %', 
            p_id_plonu, p_rolnik_id;
    END IF;

    DELETE FROM Plon WHERE id_plonu = p_id_plonu;
END;
$$;



---------- MASZYNY ---------

CREATE OR REPLACE FUNCTION dodaj_maszyne(
    p_id_rolnika INT,
    p_nazwa VARCHAR,
    p_typ VARCHAR,
    p_data_przegladu DATE,
    p_id_budynku INT
)
RETURNS VOID AS $$
DECLARE
    v_sprawdz INT;
BEGIN
    -- Walidacja daty przeglądu
    IF p_data_przegladu IS NOT NULL AND p_data_przegladu < CURRENT_DATE THEN
        RAISE EXCEPTION 'Data przeglądu nie może być w przeszłości.';
    END IF;

    -- Sprawdzenie czy budynek należy do rolnika
    IF p_id_budynku IS NOT NULL THEN
        SELECT COUNT(*) INTO v_sprawdz
        FROM Budynek
        WHERE id_budynku = p_id_budynku
          AND id_rolnika = p_id_rolnika;

        IF v_sprawdz = 0 THEN
            RAISE EXCEPTION 'Budynek nie należy do rolnika %', p_id_rolnika;
        END IF;
    END IF;

    INSERT INTO Maszyna (id_rolnika, nazwa, typ, data_przegladu, id_budynku)
    VALUES (p_id_rolnika, p_nazwa, p_typ, p_data_przegladu, p_id_budynku);
END;
$$ LANGUAGE plpgsql;



CREATE OR REPLACE FUNCTION edytuj_maszyne(
    p_id_maszyny INT,
    p_id_rolnika INT,
    p_nazwa VARCHAR,
    p_typ VARCHAR,
    p_id_budynku INT
)
RETURNS VOID AS $$
DECLARE
    v_sprawdz INT;
BEGIN
    -- Walidacja nowego budynku (jeśli podany)
    IF p_id_budynku IS NOT NULL THEN
        SELECT COUNT(*) INTO v_sprawdz
        FROM Budynek
        WHERE id_budynku = p_id_budynku
          AND id_rolnika = p_id_rolnika;

        IF v_sprawdz = 0 THEN
            RAISE EXCEPTION 'Budynek % nie należy do rolnika %', 
                p_id_budynku, p_id_rolnika;
        END IF;
    END IF;

    UPDATE Maszyna
    SET
		nazwa = COALESCE(p_nazwa, nazwa),
		typ = COALESCE(p_typ, typ),
        id_budynku = COALESCE(p_id_budynku, id_budynku)
    WHERE id_maszyny = p_id_maszyny;
END;
$$ LANGUAGE plpgsql;



CREATE OR REPLACE FUNCTION ustaw_date_przegladu(
    p_id_maszyny INT,
    p_data_przegladu DATE
)
RETURNS VOID AS $$
BEGIN
    IF p_data_przegladu < CURRENT_DATE THEN
        RAISE EXCEPTION 'Data przeglądu nie może być w przeszłości.';
    END IF;

    UPDATE Maszyna
    SET data_przegladu = p_data_przegladu
    WHERE id_maszyny = p_id_maszyny;
END;
$$ LANGUAGE plpgsql;



CREATE OR REPLACE FUNCTION usun_maszyne(
    p_id_maszyny INT
)
RETURNS VOID AS $$
DECLARE
    blokada INT;
BEGIN
    SELECT COUNT(*)
    INTO blokada
    FROM Zadanie
    WHERE id_maszyny = p_id_maszyny
      AND status NOT IN ('wykonane', 'anulowane');

    IF blokada > 0 THEN
        RAISE EXCEPTION 
            'Nie można usunąć maszyny – jest używana w aktywnych zadaniach.';
    END IF;

    DELETE FROM Maszyna WHERE id_maszyny = p_id_maszyny;
END;
$$ LANGUAGE plpgsql;


------------ MAGAZYN -------------

CREATE OR REPLACE FUNCTION dodaj_magazyn(
    p_id_rolnika INT,
    p_nazwa VARCHAR
)
RETURNS INT AS $$
DECLARE 
	v_id_magazynu INT;
BEGIN
    INSERT INTO Magazyn (id_rolnika, nazwa)
    VALUES ( p_id_rolnika, p_nazwa)
	RETURNING id_magazynu INTO v_id_magazynu;

	RETURN v_id_magazynu;
END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION edytuj_magazyn(
    p_id_magazynu INT,
    p_nazwa VARCHAR
)
RETURNS VOID AS $$
BEGIN
    UPDATE Magazyn
    SET 
		nazwa = COALESCE(p_nazwa, nazwa)
    WHERE id_magazynu = p_id_magazynu;
END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION usun_magazyn(
    p_id_magazynu INT
)
RETURNS VOID AS $$
DECLARE
    count_items INT;
BEGIN
    SELECT 
        (SELECT COUNT(*) FROM Plon WHERE id_magazynu = p_id_magazynu) +
        (SELECT COUNT(*) FROM Surowiec WHERE id_magazynu = p_id_magazynu)
    INTO count_items;

    IF count_items > 0 THEN
        RAISE EXCEPTION 'Nie można usunąć magazynu – magazyn nie jest pusty.';
    END IF;

    DELETE FROM Magazyn
    WHERE id_magazynu = p_id_magazynu;
END;
$$ LANGUAGE plpgsql;



------------- FINANSE --------------



CREATE TYPE surowiec_kupno AS (
    nazwa_surowca VARCHAR(50),
    ilosc DECIMAL(10,2),
    cena_jednostkowa DECIMAL(12,2)
);

CREATE TYPE plon_sprzedaz AS (
    id_plonu INT,
    ilosc DECIMAL(10,2),
    cena_jednostkowa DECIMAL(12,2)
);



CREATE OR REPLACE FUNCTION kup_surowce(
    p_id_rolnika INT,
    p_id_magazynu INT,
    p_opis TEXT,
    p_surowce surowiec_kupno[]
)
RETURNS VOID
LANGUAGE plpgsql
AS $$
DECLARE
    v_id_transakcji INT;
    elem surowiec_kupno;
    v_id_surowca INT;
    v_kwota DECIMAL(12,2) := 0;
BEGIN
    IF p_surowce IS NULL OR array_length(p_surowce, 1) = 0 THEN
        RAISE EXCEPTION 'Brak danych surowców do zakupu.';
    END IF;

    -- Walidacja magazynu
    IF NOT EXISTS (
        SELECT 1 FROM Magazyn 
        WHERE id_magazynu = p_id_magazynu
          AND id_rolnika = p_id_rolnika
    ) THEN
        RAISE EXCEPTION 'Magazyn % nie należy do rolnika %',
            p_id_magazynu, p_id_rolnika;
    END IF;

    -- Tworzenie transakcji
    INSERT INTO Transakcja(id_rolnika, typ, opis, kwota)
    VALUES (p_id_rolnika, 'kupno', p_opis, 0)
    RETURNING id_transakcji INTO v_id_transakcji;

    -- Przetwarzanie surowców
    FOREACH elem IN ARRAY p_surowce LOOP

        -- Sprawdzenie czy istnieje surowiec o tej nazwie
        SELECT id_surowca INTO v_id_surowca
        FROM Surowiec
        WHERE nazwa = elem.nazwa_surowca
          AND id_magazynu = p_id_magazynu;

        IF v_id_surowca IS NULL THEN
            -- Tworzenie nowego surowca
            INSERT INTO Surowiec(id_magazynu, nazwa, ilosc)
            VALUES (p_id_magazynu, elem.nazwa_surowca, elem.ilosc)
            RETURNING id_surowca INTO v_id_surowca;
        ELSE
            -- Aktualizacja istniejącego
            UPDATE Surowiec
            SET ilosc = ilosc + elem.ilosc
            WHERE id_surowca = v_id_surowca;
        END IF;

        -- Pozycja N-M
        INSERT INTO Transakcja_Surowiec(id_transakcji, id_surowca, ilosc, cena_jednostkowa)
        VALUES (v_id_transakcji, v_id_surowca, elem.ilosc, elem.cena_jednostkowa);

        -- Liczymy pełną kwotę
        v_kwota := v_kwota + (elem.ilosc * elem.cena_jednostkowa);
    END LOOP;

    -- Aktualizacja pełnej kwoty transakcji
    UPDATE Transakcja
    SET kwota = v_kwota
    WHERE id_transakcji = v_id_transakcji;
END;
$$;





CREATE OR REPLACE FUNCTION sprzedaj_plony(
    p_id_rolnika INT,
    p_id_magazynu INT,
    p_opis TEXT,
    p_plony plon_sprzedaz[]
)
RETURNS VOID
LANGUAGE plpgsql
AS $$
DECLARE
    elem plon_sprzedaz;
    v_id_transakcji INT;
    v_magazyn_plon INT;
    v_kwota DECIMAL(12,2) := 0;
BEGIN
    IF p_plony IS NULL OR array_length(p_plony, 1) = 0 THEN
        RAISE EXCEPTION 'Brak danych plonów do sprzedaży.';
    END IF;

    -- Walidacja magazynu
    IF NOT EXISTS (
        SELECT 1 FROM Magazyn 
        WHERE id_magazynu = p_id_magazynu
          AND id_rolnika = p_id_rolnika
    ) THEN
        RAISE EXCEPTION 'Magazyn % nie należy do rolnika %',
            p_id_magazynu, p_id_rolnika;
    END IF;

    -- Tworzenie transakcji
    INSERT INTO Transakcja(id_rolnika, typ, opis, kwota)
    VALUES (p_id_rolnika, 'sprzedaz', p_opis, 0)
    RETURNING id_transakcji INTO v_id_transakcji;

    FOREACH elem IN ARRAY p_plony LOOP
    
        SELECT id_magazynu INTO v_magazyn_plon
        FROM Plon
        WHERE id_plonu = elem.id_plonu;

        IF v_magazyn_plon IS NULL THEN
            RAISE EXCEPTION 'Plon % nie istnieje.', elem.id_plonu;
        END IF;

        IF v_magazyn_plon != p_id_magazynu THEN
            RAISE EXCEPTION 'Plon % nie znajduje się w magazynie %', 
                elem.id_plonu, p_id_magazynu;
        END IF;

        -- Sprawdzenie ilości
        PERFORM 1 FROM Plon
        WHERE id_plonu = elem.id_plonu
          AND ilosc >= elem.ilosc;

        IF NOT FOUND THEN
            RAISE EXCEPTION 'Za mało plonu %. Dostępne mniej niż %.',
                elem.id_plonu, elem.ilosc;
        END IF;

        -- Aktualizacja ilości
        UPDATE Plon
        SET ilosc = ilosc - elem.ilosc
        WHERE id_plonu = elem.id_plonu;

        

        -- Relacja N-M
        INSERT INTO Transakcja_Plon(id_transakcji, id_plonu, ilosc, cena_jednostkowa)
        VALUES (v_id_transakcji, elem.id_plonu, elem.ilosc, elem.cena_jednostkowa);

        v_kwota := v_kwota + (elem.ilosc * elem.cena_jednostkowa);
    END LOOP;

    -- Aktualizacja kwoty końcowej
    UPDATE Transakcja
    SET kwota = v_kwota
    WHERE id_transakcji = v_id_transakcji;
END;
$$;



-------- BUDYNKI --------


CREATE OR REPLACE FUNCTION gospodarka.edytuj_budynek(p_id_rolnika integer, p_id_budynku integer, p_nazwa character varying DEFAULT NULL::character varying, p_typ character varying DEFAULT NULL::character varying, p_pojemnosc integer DEFAULT NULL::integer)
 RETURNS void
 LANGUAGE plpgsql
AS $function$
DECLARE
    v_exists BOOLEAN;
BEGIN
    -- Sprawdzenie czy budynek należy do rolnika
    SELECT EXISTS(
        SELECT 1 FROM Budynek 
        WHERE id_budynku = p_id_budynku
          AND id_rolnika = p_id_rolnika
    ) INTO v_exists;

    IF NOT v_exists THEN
        RAISE EXCEPTION 'Budynek % nie należy do rolnika %', p_id_budynku, p_id_rolnika;
    END IF;

    UPDATE Budynek
    SET 
        nazwa = COALESCE(p_nazwa, nazwa),
        typ = COALESCE(p_typ, typ),
        pojemnosc = COALESCE(p_pojemnosc, pojemnosc)
    WHERE id_budynku = p_id_budynku;
END;
$function$
;


CREATE OR REPLACE FUNCTION gospodarka.usun_budynek(p_id_rolnika integer, p_id_budynku integer)
 RETURNS void
 LANGUAGE plpgsql
AS $function$
DECLARE
    v_exists BOOLEAN;
    v_zwierzeta INT;
    v_maszyny INT;
BEGIN
    -- Autoryzacja
    SELECT EXISTS(
        SELECT 1 FROM Budynek
        WHERE id_budynku = p_id_budynku
          AND id_rolnika = p_id_rolnika
    ) INTO v_exists;

    IF NOT v_exists THEN
        RAISE EXCEPTION 'Budynek % nie należy do rolnika %', p_id_budynku, p_id_rolnika;
    END IF;

    -- Sprawdzenie zwierząt w budynku
    SELECT COUNT(*) INTO v_zwierzeta
    FROM Zwierze
    WHERE id_budynku = p_id_budynku;

    IF v_zwierzeta > 0 THEN
        RAISE EXCEPTION 'Nie można usunąć budynku %, są w nim zwierzęta (% szt.)', 
            p_id_budynku, v_zwierzeta;
    END IF;

    -- Sprawdzenie maszyn w budynku
    SELECT COUNT(*) INTO v_maszyny
    FROM Maszyna
    WHERE id_budynku = p_id_budynku;

    IF v_maszyny > 0 THEN
        RAISE EXCEPTION 'Nie można usunąć budynku %, są w nim maszyny (% szt.)', 
            p_id_budynku, v_maszyny;
    END IF;

    DELETE FROM Budynek
    WHERE id_budynku = p_id_budynku;
END;
$function$
;


CREATE OR REPLACE FUNCTION gospodarka.dodaj_budynek(p_id_rolnika integer, p_nazwa character varying, p_typ character varying DEFAULT NULL::character varying, p_pojemnosc integer DEFAULT NULL::integer)
 RETURNS integer
 LANGUAGE plpgsql
AS $function$
DECLARE 
	v_id_budynek INT;
BEGIN

    INSERT INTO Budynek(nazwa, typ, pojemnosc, id_rolnika)
    VALUES(p_nazwa, p_typ, p_pojemnosc, p_id_rolnika)
	RETURNING id_budynku INTO v_id_budynek;

	RETURN v_id_budynek;
END;
$function$
;



------------- POLE ------------


CREATE OR REPLACE FUNCTION rolnik_dodaj_pole(
    p_id_rolnika INT,
    p_nazwa VARCHAR,
    p_powierzchnia DECIMAL
)
RETURNS TEXT AS $$
BEGIN
    IF p_powierzchnia <= 0 THEN
        RETURN 'Błąd: Powierzchnia musi być większa od 0.';
    END IF;

    INSERT INTO Pole (nazwa, powierzchnia, id_rolnika)
    VALUES (p_nazwa, p_powierzchnia, p_id_rolnika);

    RETURN 'Pole zostało dodane.';
END;
$$ LANGUAGE plpgsql;




CREATE OR REPLACE FUNCTION rolnik_edytuj_pole(
    p_id_rolnika INT,
    p_id_pola INT,
    p_nazwa VARCHAR DEFAULT NULL,
    p_powierzchnia DECIMAL DEFAULT NULL
)
RETURNS TEXT AS $$
DECLARE
    v_exists INT;
    v_uprawy INT;
BEGIN
    -- Sprawdzenie czy pole należy do rolnika
    SELECT COUNT(*) INTO v_exists
    FROM Pole
    WHERE id_pola = p_id_pola AND id_rolnika = p_id_rolnika;

    IF v_exists = 0 THEN
        RETURN 'Błąd: Pole nie istnieje lub nie należy do rolnika.';
    END IF;

    -- Sprawdzenie aktywnej uprawy
    SELECT COUNT(*) INTO v_uprawy
    FROM Uprawa
    WHERE id_pola = p_id_pola
      AND data_siewu <= CURRENT_DATE
      AND (data_zbioru IS NULL OR data_zbioru > CURRENT_DATE);

    IF v_uprawy > 0 THEN
        RETURN 'Błąd: Pole ma aktywną uprawę — edycja zabroniona.';
    END IF;

    -- Wykonanie aktualizacji
    UPDATE Pole
    SET 
        nazwa = COALESCE(p_nazwa, nazwa),
        powierzchnia = COALESCE(p_powierzchnia, powierzchnia)
    WHERE id_pola = p_id_pola;

    RETURN 'Pole zostało zaktualizowane.';
END;
$$ LANGUAGE plpgsql;



CREATE OR REPLACE FUNCTION rolnik_usun_pole(
    p_id_rolnika INT,
    p_id_pola INT
)
RETURNS TEXT AS $$
DECLARE
    v_exists INT;
    v_uprawy INT;
BEGIN
    -- Sprawdzenie czy pole należy do rolnika
    SELECT COUNT(*) INTO v_exists
    FROM Pole
    WHERE id_pola = p_id_pola AND id_rolnika = p_id_rolnika;

    IF v_exists = 0 THEN
        RETURN 'Błąd: Pole nie istnieje lub nie należy do rolnika.';
    END IF;

    -- Sprawdzenie aktywnych upraw
    SELECT COUNT(*) INTO v_uprawy
    FROM Uprawa
    WHERE id_pola = p_id_pola
      AND data_siewu <= CURRENT_DATE
      AND (data_zbioru IS NULL OR data_zbioru > CURRENT_DATE);

    IF v_uprawy > 0 THEN
        RETURN 'Błąd: Nie można usunąć pola z aktywną uprawą.';
    END IF;

    DELETE FROM Pole
    WHERE id_pola = p_id_pola;

    RETURN 'Pole zostało usunięte.';
END;
$$ LANGUAGE plpgsql;



----------- ADMIN ----------


CREATE OR REPLACE FUNCTION admin_dodaj_rolnika(
    p_login VARCHAR,
    p_haslo VARCHAR
)
RETURNS TEXT AS $$
DECLARE
    v_id INT;
BEGIN
    -- unikalny login
    IF EXISTS (SELECT 1 FROM Uzytkownik WHERE login = p_login) THEN
        RETURN 'Błąd: Login już istnieje.';
    END IF;

    INSERT INTO Uzytkownik(login, haslo, rola, aktywny)
    VALUES (p_login, p_haslo, 'rolnik', TRUE)
    RETURNING id_uzytkownika INTO v_id;

    RETURN 'Rolnik został dodany. ID=' || v_id;
END;
$$ LANGUAGE plpgsql;




CREATE OR REPLACE FUNCTION admin_dodaj_pracownika(
    p_login VARCHAR,
    p_haslo VARCHAR,
    p_imie VARCHAR,
    p_nazwisko VARCHAR,
    p_id_rolnika INT
)
RETURNS TEXT AS $$
DECLARE
    v_id INT;
BEGIN
    -- unikalny login
    IF EXISTS (SELECT 1 FROM Uzytkownik WHERE login = p_login) THEN
        RETURN 'Błąd: Login już istnieje.';
    END IF;

    -- czy rolnik istnieje?
    IF NOT EXISTS (
        SELECT 1 FROM Uzytkownik WHERE id_uzytkownika = p_id_rolnika AND rola = 'rolnik'
    ) THEN
        RETURN 'Błąd: Podany rolnik nie istnieje.';
    END IF;

    INSERT INTO Uzytkownik(login, haslo, rola, aktywny)
    VALUES (p_login, p_haslo, 'pracownik', TRUE)
    RETURNING id_uzytkownika INTO v_id;

    INSERT INTO Pracownik(id_uzytkownika, id_rolnika, imie, nazwisko, data_zatrudnienia)
    VALUES (v_id, p_id_rolnika, p_imie, p_nazwisko, NULL);

    RETURN 'Pracownik został dodany. ID=' || v_id;
END;
$$ LANGUAGE plpgsql;




CREATE OR REPLACE FUNCTION admin_zmien_dane_logowania(
    p_id_uzytkownika INT,
    p_login VARCHAR DEFAULT NULL,
    p_haslo VARCHAR DEFAULT NULL
)
RETURNS TEXT AS $$
DECLARE
    v_stary_login VARCHAR;
BEGIN
    SELECT login INTO v_stary_login
    FROM Uzytkownik
    WHERE id_uzytkownika = p_id_uzytkownika;

    IF NOT FOUND THEN
        RETURN 'Błąd: Użytkownik nie istnieje.';
    END IF;

    -- walidacja loginu
    IF p_login IS NOT NULL AND p_login <> v_stary_login THEN
        IF EXISTS (SELECT 1 FROM Uzytkownik WHERE login = p_login) THEN
            RETURN 'Błąd: Login już istnieje.';
        END IF;
    END IF;

    UPDATE Uzytkownik
    SET 
        login = COALESCE(p_login, login),
        haslo = COALESCE(p_haslo, haslo)
    WHERE id_uzytkownika = p_id_uzytkownika;

    RETURN 'Dane logowania zostały zaktualizowane.';
END;
$$ LANGUAGE plpgsql;



CREATE OR REPLACE FUNCTION admin_dezaktywuj_konto(
    p_id_uzytkownika INT
)
RETURNS TEXT AS $$
DECLARE
    v_rola VARCHAR;
	v_aktywny BOOLEAN;
BEGIN
    SELECT rola, aktywny INTO v_rola, v_aktywny
    FROM Uzytkownik
    WHERE id_uzytkownika = p_id_uzytkownika;

    IF NOT FOUND THEN
        RETURN 'Błąd: Użytkownik nie istnieje.';
    END IF;

    IF v_rola = 'admin' THEN
        RETURN 'Błąd: Konto admina nie może być dezaktywowane.';
    END IF;

	IF v_aktywny = FALSE THEN
		RETURN 'Błąd: Konto już jest nie aktywne.';
	END IF;

    UPDATE Uzytkownik
    SET aktywny = FALSE
    WHERE id_uzytkownika = p_id_uzytkownika;

    RETURN 'Konto zostało dezaktywowane.';
END;
$$ LANGUAGE plpgsql;


CREATE OR REPLACE FUNCTION admin_aktywuj_konto(
    p_id_uzytkownika INT
)
RETURNS TEXT AS $$
DECLARE
    v_rola VARCHAR;
	v_aktywny BOOLEAN;
BEGIN
    SELECT rola, aktywny INTO v_rola, v_aktywny
    FROM Uzytkownik
    WHERE id_uzytkownika = p_id_uzytkownika;
    
	IF NOT FOUND THEN
        RETURN 'Błąd: Użytkownik nie istnieje.';
    END IF;

	IF v_aktywny = TRUE THEN
		RETURN 'Błąd: Konto już jest aktywne.';
	END IF;

    UPDATE Uzytkownik
    SET aktywny = TRUE
    WHERE id_uzytkownika = p_id_uzytkownika;

    RETURN 'Konto zostało ponownie aktywowane.';
END;
$$ LANGUAGE plpgsql;




CREATE OR REPLACE FUNCTION admin_usun_uzytkownika(
    p_id_uzytkownika INT
)
RETURNS TEXT AS $$
DECLARE
    v_rola VARCHAR;
BEGIN
    SELECT rola INTO v_rola
    FROM Uzytkownik
    WHERE id_uzytkownika = p_id_uzytkownika;

    IF NOT FOUND THEN
        RETURN 'Błąd: Użytkownik nie istnieje.';
    END IF;

    IF v_rola = 'admin' THEN
        RETURN 'Błąd: Nie można usuwać kont typu admin.';
    END IF;

    DELETE FROM Uzytkownik
    WHERE id_uzytkownika = p_id_uzytkownika;

    RETURN 'Użytkownik został usunięty.';
END;
$$ LANGUAGE plpgsql;



-------------- ZADANIA --------------


CREATE OR REPLACE FUNCTION edytuj_zadanie(
    p_id_zadania INT,
    p_id_uzytkownika INT,
    p_nazwa VARCHAR DEFAULT NULL,
    p_opis TEXT DEFAULT NULL,
    p_id_pracownika INT DEFAULT NULL,
    p_id_uprawy INT DEFAULT NULL,
    p_id_pola INT DEFAULT NULL,
    p_id_budynku INT DEFAULT NULL,
    p_id_maszyny INT DEFAULT NULL,
    p_id_zwierzecia INT DEFAULT NULL,
    p_data_start DATE DEFAULT NULL
)
RETURNS VOID AS $$
DECLARE
    v_id_rolnika INT;
    v_id_pracownika INT;
    v_status VARCHAR;
BEGIN
    ---------------------------------------------------
    -- 1. Pobranie danych zadania
    ---------------------------------------------------
    SELECT id_rolnika, id_pracownika, status
    INTO v_id_rolnika, v_id_pracownika, v_status
    FROM Zadanie
    WHERE id_zadania = p_id_zadania;

    IF v_id_rolnika IS NULL THEN
        RAISE EXCEPTION 'Zadanie % nie istnieje', p_id_zadania;
    END IF;

    ---------------------------------------------------
    -- 2. Sprawdzenie, czy zadanie nie jest wykonane
    ---------------------------------------------------
    IF v_status = 'wykonane' THEN
        RAISE EXCEPTION 'Nie można edytować zadania %: status = wykonane', p_id_zadania;
    END IF;

    ---------------------------------------------------
    -- 3. Autoryzacja użytkownika
    ---------------------------------------------------
    IF p_id_uzytkownika <> v_id_rolnika 
       AND p_id_uzytkownika <> v_id_pracownika THEN
        RAISE EXCEPTION 
            'Użytkownik % nie ma uprawnień do edycji zadania %',
            p_id_uzytkownika, p_id_zadania;
    END IF;

    ---------------------------------------------------
    -- 4. Walidacja zasobów (czy należą do rolnika)
    ---------------------------------------------------
    IF p_id_pracownika IS NOT NULL THEN
        IF NOT EXISTS (
            SELECT 1 FROM Pracownik WHERE id_pracownika = p_id_pracownika AND id_rolnika = v_id_rolnika
        ) THEN
            RAISE EXCEPTION 'Pracownik % nie należy do rolnika %', p_id_pracownika, v_id_rolnika;
        END IF;
    END IF;

    IF p_id_pola IS NOT NULL THEN
        IF NOT EXISTS (
            SELECT 1 FROM Pole WHERE id_pola = p_id_pola AND id_rolnika = v_id_rolnika
        ) THEN
            RAISE EXCEPTION 'Pole % nie należy do rolnika %', p_id_pola, v_id_rolnika;
        END IF;
    END IF;

    IF p_id_budynku IS NOT NULL THEN
        IF NOT EXISTS (
            SELECT 1 FROM Budynek WHERE id_budynku = p_id_budynku AND id_rolnika = v_id_rolnika
        ) THEN
            RAISE EXCEPTION 'Budynek % nie należy do rolnika %', p_id_budynku, v_id_rolnika;
        END IF;
    END IF;

    IF p_id_maszyny IS NOT NULL THEN
        IF NOT EXISTS (
            SELECT 1 FROM Maszyna WHERE id_maszyny = p_id_maszyny AND id_rolnika = v_id_rolnika
        ) THEN
            RAISE EXCEPTION 'Maszyna % nie należy do rolnika %', p_id_maszyny, v_id_rolnika;
        END IF;
    END IF;

    IF p_id_zwierzecia IS NOT NULL THEN
        IF NOT EXISTS (
            SELECT 1 FROM Zwierze WHERE id_zwierzecia = p_id_zwierzecia AND id_rolnika = v_id_rolnika
        ) THEN
            RAISE EXCEPTION 'Zwierzę % nie należy do rolnika %', p_id_zwierzecia, v_id_rolnika;
        END IF;
    END IF;

    ---------------------------------------------------
    -- 5. Aktualizacja zadania (tylko pola != NULL)
    ---------------------------------------------------
    UPDATE Zadanie
    SET
        nazwa = COALESCE(p_nazwa, nazwa),
        opis = COALESCE(p_opis, opis),
        id_pracownika = COALESCE(p_id_pracownika, id_pracownika),
        id_uprawy = COALESCE(p_id_uprawy, id_uprawy),
        id_pola = COALESCE(p_id_pola, id_pola),
        id_budynku = COALESCE(p_id_budynku, id_budynku),
        id_maszyny = COALESCE(p_id_maszyny, id_maszyny),
        id_zwierzecia = COALESCE(p_id_zwierzecia, id_zwierzecia),
        data_start = COALESCE(p_data_start, data_start)
    WHERE id_zadania = p_id_zadania;

END;
$$ LANGUAGE plpgsql;




CREATE OR REPLACE FUNCTION usun_zadanie(
    p_id_zadania INT,
    p_id_uzytkownika INT
)
RETURNS VOID AS $$
DECLARE
    v_id_rolnika INT;
    v_status VARCHAR;
BEGIN
    ---------------------------------------------------
    -- 1. Pobranie danych zadania
    ---------------------------------------------------
    SELECT id_rolnika, status
    INTO v_id_rolnika, v_status
    FROM Zadanie
    WHERE id_zadania = p_id_zadania;

    IF v_id_rolnika IS NULL THEN
        RAISE EXCEPTION 'Zadanie % nie istnieje', p_id_zadania;
    END IF;

    ---------------------------------------------------
    -- 2. Autoryzacja
    ---------------------------------------------------
    IF p_id_uzytkownika <> v_id_rolnika THEN
        RAISE EXCEPTION 
            'Użytkownik % nie ma uprawnień do usunięcia zadania %',
            p_id_uzytkownika, p_id_zadania;
    END IF;

    ---------------------------------------------------
    -- 3. Blokada usuwania wykonanego zadania
    ---------------------------------------------------
    IF v_status = 'wykonane' THEN
        RAISE EXCEPTION 
            'Nie można usunąć zadania %, ponieważ jest wykonane.',
            p_id_zadania;
    END IF;

    ---------------------------------------------------
    -- 4. Usuwanie powiązań N-M
    ---------------------------------------------------
    DELETE FROM Zadanie_Surowiec
    WHERE id_zadania = p_id_zadania;

    DELETE FROM Zadanie_Plon
    WHERE id_zadania = p_id_zadania;

    ---------------------------------------------------
    -- 5. Usunięcie zadania
    ---------------------------------------------------
    DELETE FROM Zadanie
    WHERE id_zadania = p_id_zadania;

END;
$$ LANGUAGE plpgsql;



CREATE OR REPLACE FUNCTION utworz_zadanie(
    p_nazwa TEXT,
    p_opis TEXT,
    p_id_rolnika INT,
    p_id_pracownika INT DEFAULT NULL,
    p_id_uprawy INT DEFAULT NULL,
    p_id_maszyny INT DEFAULT NULL,
    p_id_budynku INT DEFAULT NULL,
    p_id_zwierzecia INT DEFAULT NULL,
    p_id_pola INT DEFAULT NULL,
    p_data_start DATE DEFAULT NULL
) RETURNS INT
LANGUAGE plpgsql AS
$$
DECLARE
    v_new_id INT;
    v_tmp INT;
BEGIN
    -- Walidacja: rolnik istnieje i ma rolę 'rolnik'
    SELECT id_uzytkownika INTO v_tmp FROM Uzytkownik WHERE id_uzytkownika = p_id_rolnika AND rola = 'rolnik';
    IF NOT FOUND THEN
        RAISE EXCEPTION 'Nie znaleziono rolnika o id % lub nie ma roli ''rolnik''', p_id_rolnika;
    END IF;

    -- Walidacja: pracownik (jeśli podany) istnieje i należy do tego rolnika
    IF p_id_pracownika IS NOT NULL THEN
        SELECT id_pracownika INTO v_tmp FROM Pracownik WHERE id_pracownika = p_id_pracownika AND id_rolnika = p_id_rolnika;
        IF NOT FOUND THEN
            RAISE EXCEPTION 'Pracownik id % nie istnieje lub nie jest przypisany do rolnika id %', p_id_pracownika, p_id_rolnika;
        END IF;
    END IF;

    -- Walidacje dla innych encji (jeśli podane) — upewniamy się, że zasoby należą do rolnika (tam gdzie ma to sens)
    IF p_id_uprawy IS NOT NULL THEN
        SELECT u.id_uzytkownika INTO v_tmp
        FROM Uprawa up JOIN Pole po ON up.id_pola = po.id_pola
        JOIN Uzytkownik u ON po.id_rolnika = u.id_uzytkownika
        WHERE up.id_uprawy = p_id_uprawy AND po.id_rolnika = p_id_rolnika;
        IF NOT FOUND THEN
            RAISE EXCEPTION 'Uprawa id % nie istnieje lub nie nalezy do rolnika id %', p_id_uprawy, p_id_rolnika;
        END IF;
    END IF;

    IF p_id_pola IS NOT NULL THEN
        SELECT id_pola INTO v_tmp FROM Pole WHERE id_pola = p_id_pola AND id_rolnika = p_id_rolnika;
        IF NOT FOUND THEN
            RAISE EXCEPTION 'Pole id % nie istnieje lub nie nalezy do rolnika id %', p_id_pola, p_id_rolnika;
        END IF;
    END IF;

    IF p_id_maszyny IS NOT NULL THEN
        SELECT id_maszyny INTO v_tmp FROM Maszyna WHERE id_maszyny = p_id_maszyny AND id_rolnika = p_id_rolnika;
        IF NOT FOUND THEN
            RAISE EXCEPTION 'Maszyna id % nie istnieje lub nie nalezy do rolnika id %', p_id_maszyny, p_id_rolnika;
        END IF;
    END IF;

    IF p_id_budynku IS NOT NULL THEN
        SELECT id_budynku INTO v_tmp FROM Budynek WHERE id_budynku = p_id_budynku AND id_rolnika = p_id_rolnika;
        IF NOT FOUND THEN
            RAISE EXCEPTION 'Budynek id % nie istnieje lub nie nalezy do rolnika id %', p_id_budynku, p_id_rolnika;
        END IF;
    END IF;

    IF p_id_zwierzecia IS NOT NULL THEN
        SELECT id_zwierzecia INTO v_tmp FROM Zwierze WHERE id_zwierzecia = p_id_zwierzecia AND id_rolnika = p_id_rolnika;
        IF NOT FOUND THEN
            RAISE EXCEPTION 'Zwierze id % nie istnieje lub nie nalezy do rolnika id %', p_id_zwierzecia, p_id_rolnika;
        END IF;
    END IF;


    -- Wstawienie zadania
    INSERT INTO Zadanie (
        nazwa, opis, id_rolnika, id_pracownika, id_uprawy, id_maszyny, id_budynku, id_zwierzecia, id_pola, data_start, status
    )
    VALUES (
        p_nazwa, p_opis, p_id_rolnika, p_id_pracownika, p_id_uprawy, p_id_maszyny, p_id_budynku, p_id_zwierzecia, p_id_pola, p_data_start, 'planowane'
    )
    RETURNING id_zadania INTO v_new_id;

    RETURN v_new_id;
END;
$$;



CREATE TYPE pozyskany_plon AS (
    nazwa_plonu VARCHAR(50),
    ilosc NUMERIC
);

CREATE TYPE zuzycie_surowca AS (
    id_surowca INT,
    ilosc NUMERIC
);



CREATE OR REPLACE FUNCTION wykonaj_zadanie(
    p_id_zadania       INT,
    p_id_uzytkownika   INT, -- id_rolnika lub id_pracownika (nie pracowniczy id_uzytkownika)
    p_id_magazynu      INT,
    p_surowce          zuzycie_surowca[] DEFAULT NULL,
    p_plony            pozyskany_plon[]  DEFAULT NULL
)
RETURNS VOID
LANGUAGE plpgsql
AS $$
DECLARE
    v_id_rolnika INT;
    v_id_pracownika INT;
    v_magazyn INT;
    v_id_uprawy INT;
    v_id_zwierzecia INT;
    v_nazwa_zadania VARCHAR;

    elem_surowiec zuzycie_surowca;
    elem_plon pozyskany_plon;

    v_available NUMERIC;
    v_id_plonu INT;
BEGIN

    -------------------------------------------------------
    -- 1. Pobranie danych zadania
    -------------------------------------------------------
    SELECT id_uprawy, id_zwierzecia, id_rolnika, id_pracownika, nazwa
    INTO v_id_uprawy, v_id_zwierzecia, v_id_rolnika, v_id_pracownika, v_nazwa_zadania
    FROM Zadanie WHERE id_zadania = p_id_zadania;

    IF v_id_rolnika IS NULL THEN
        RAISE EXCEPTION 'Zadanie % nie istnieje', p_id_zadania;
    END IF;

    -------------------------------------------------------
    -- 2. Autoryzacja
    -------------------------------------------------------
    IF p_id_uzytkownika NOT IN (v_id_rolnika, v_id_pracownika) THEN
        RAISE EXCEPTION 'Brak uprawnień do wykonania zadania %', p_id_zadania;
    END IF;

    -------------------------------------------------------
	-- 3. Walidacja magazynu
	-------------------------------------------------------
	SELECT id_magazynu INTO v_magazyn
	FROM Magazyn
		WHERE id_magazynu = p_id_magazynu
  		AND id_rolnika = v_id_rolnika;

	IF v_magazyn IS NULL THEN
    	RAISE EXCEPTION 'Magazyn % nie należy do rolnika %', 
        p_id_magazynu, v_id_rolnika;
	END IF;

    -------------------------------------------------------
    -- 4. Aktualizacja statusu zadania
    -------------------------------------------------------
    UPDATE Zadanie
    SET status = 'wykonane', data_koniec = NOW()
    WHERE id_zadania = p_id_zadania;

    -------------------------------------------------------
    -- 5. ZUŻYCIE WIELU SUROWCÓW
    -------------------------------------------------------
    IF p_surowce IS NOT NULL THEN
        FOREACH elem_surowiec IN ARRAY p_surowce LOOP

            SELECT ilosc INTO v_available
            FROM Surowiec
            WHERE id_surowca = elem_surowiec.id_surowca
              AND id_magazynu = v_magazyn;

            IF v_available IS NULL THEN
                RAISE EXCEPTION 'Surowiec % nie istnieje w magazynie!',
                       elem_surowiec.id_surowca;
            END IF;

            IF v_available < elem_surowiec.ilosc THEN
                RAISE EXCEPTION 'Za mało surowca %. Dostępne %, potrzebne %',
                       elem_surowiec.id_surowca,
                       v_available,
                       elem_surowiec.ilosc;
            END IF;

            -- aktualizacja magazynu
            UPDATE Surowiec
            SET ilosc = ilosc - elem_surowiec.ilosc
            WHERE id_surowca = elem_surowiec.id_surowca;

            -- zapis N-M
            INSERT INTO Zadanie_Surowiec (id_zadania, id_surowca, ilosc_uzyta)
            VALUES (p_id_zadania, elem_surowiec.id_surowca, elem_surowiec.ilosc);
        END LOOP;
    END IF;

    -------------------------------------------------------
    -- 6. POZYSKANIE WIELU PLONÓW (wpisywane po NAZWIE, nie ID!)
    -------------------------------------------------------
    IF p_plony IS NOT NULL THEN
		IF v_id_uprawy IS NULL AND v_id_zwierzecia is NULL THEN
			RAISE EXCEPTION 'Brark przypisanego zwierzęcia lub uprawy. Nie da się stworzyć plonu';
		END IF;

		FOREACH elem_plon IN ARRAY p_plony LOOP

            -- najpierw spróbuj znaleźć po nazwie i magazynie
            SELECT id_plonu INTO v_id_plonu
            FROM Plon
            WHERE LOWER(nazwa) = LOWER(elem_plon.nazwa_plonu)
              AND id_magazynu = v_magazyn;

            IF v_id_plonu IS NULL THEN
                -- plon nie istnieje → tworzymy nowy
                INSERT INTO Plon(id_uprawy, id_zwierzecia, id_magazynu, ilosc, nazwa, data_pozyskania)
                VALUES (
                    v_id_uprawy,
                    v_id_zwierzecia,
                    v_magazyn,
                    elem_plon.ilosc,
                    elem_plon.nazwa_plonu,
                    CURRENT_DATE
                )
                RETURNING id_plonu INTO v_id_plonu;

            ELSE
                -- plon istnieje → zwiększamy ilość
                UPDATE Plon
                SET ilosc = ilosc + elem_plon.ilosc, data_pozyskania = CURRENT_DATE

                WHERE id_plonu = v_id_plonu;
            END IF;

            -- zapis N-M
            INSERT INTO Zadanie_Plon(id_zadania, id_plonu, ilosc_pozyskana)
            VALUES (p_id_zadania, v_id_plonu, elem_plon.ilosc);

        END LOOP;
    END IF;

    -------------------------------------------------------
    -- 7. Automatyczne ustawienie daty siewu
    -------------------------------------------------------
    IF v_id_uprawy IS NOT NULL AND v_nazwa_zadania ILIKE 'Siew%' THEN
        UPDATE Uprawa SET data_siewu = CURRENT_DATE
        WHERE id_uprawy = v_id_uprawy;
    END IF;

    -------------------------------------------------------
    -- 8. Automatyczne ustawienie daty zbioru
    -------------------------------------------------------
    IF v_id_uprawy IS NOT NULL AND p_plony IS NOT NULL THEN
        UPDATE Uprawa SET data_zbioru = CURRENT_DATE
        WHERE id_uprawy = v_id_uprawy;
    END IF;

END;
$$;




CREATE OR REPLACE FUNCTION zmien_status_zadania(
    p_id_zadania INT,
    p_id_uzytkownika INT, -- id_uzytkownika (nie id_pracownika)
    p_nowystatus VARCHAR
)
RETURNS TEXT AS $$
DECLARE
    v_id_rolnika INT;
    v_id_pracownika INT;
    v_id_uzytkownika_pracownika INT;
    v_rola VARCHAR(20);
BEGIN
    -- Pobranie danych zadania
    SELECT id_rolnika, id_pracownika
    INTO v_id_rolnika, v_id_pracownika
    FROM Zadanie
    WHERE id_zadania = p_id_zadania;

    IF NOT FOUND THEN
        RETURN 'Błąd: Zadanie o podanym ID nie istnieje.';
    END IF;

    -- Pobranie roli użytkownika
    SELECT rola INTO v_rola
    FROM Uzytkownik
    WHERE id_uzytkownika = p_id_uzytkownika;

    IF v_rola IS NULL THEN
        RETURN 'Błąd: Użytkownik nie istnieje.';
    END IF;

    -- Uprawnienia
    IF v_rola = 'rolnik' THEN
        IF v_id_rolnika <> p_id_uzytkownika THEN
            RETURN 'Błąd: Rolnik nie może zmienić statusu zadania należącego do innego rolnika.';
        END IF;
    ELSE
        
        -- Jeśli zadanie ma przypisanego pracownika – tylko on może zmienić status
        IF v_id_pracownika IS NOT NULL AND v_id_uzytkownika_pracownika <> p_id_uzytkownika THEN
            RETURN 'Błąd: Pracownik nie jest przypisany do tego zadania.';
        END IF;
    END IF;

    -- Blokada ustawiania statusu "wykonane"
    IF p_nowystatus = 'wykonane' THEN
        RETURN 'Błąd: Status "wykonane" ustawia tylko funkcja wykonaj_zadanie().';
    END IF;

    -- Sprawdzenie poprawnych statusów
    IF p_nowystatus NOT IN ('planowane','w_trakcie','anulowane') THEN
        RETURN 'Błąd: Niedozwolony status. Do wyboru: planowane, w_trakcie, anulowane';
    END IF;

    -- Aktualizacja statusu
    UPDATE Zadanie
    SET status = p_nowystatus,
        data_start = CASE
                        WHEN p_nowystatus = 'w_trakcie' THEN CURRENT_DATE
                        ELSE data_start
                     END
    WHERE id_zadania = p_id_zadania;

    RETURN 'Status zadania został zmieniony na: ' || p_nowystatus;
END;
$$ LANGUAGE plpgsql;





