/**
 * @file test_text_input_field.cpp
 * @brief Tests unitaires du champ de saisie de texte de l'éditeur (LOT-15, EX-EDIT-009).
 */

#include <gtest/gtest.h>

#include "HMI/Editor/TextInputField.h"
#include "HMI/Input/InputState.h"

/**
 * @brief Les caractères tapés s'accumulent dans le texte du champ, dans l'ordre de saisie.
 * \castest{<b>Les caractères tapés s'accumulent dans le texte du champ, dans l'ordre de
 * saisie.</b><br/>
 * \tcat Unitaire · Text Input Field<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Les caractères tapés s'accumulent dans le texte du champ, dans l'ordre de saisie.
 * }
 */
TEST(TextInputFieldTest, CaracteresTapesSAccumulent) {
    hmi::InputState input;
    hmi::TextInputField field;

    input.beginFrame();
    input.onCharTyped(L'N');
    input.onCharTyped(L'1');
    field.update(input);

    EXPECT_EQ(field.text(), "N1");
    EXPECT_FALSE(field.confirmed());
    EXPECT_FALSE(field.cancelled());
}

/**
 * @brief Retour arrière retire le dernier caractère du texte pré-rempli.
 * \castest{<b>Retour arrière retire le dernier caractère du texte pré-rempli.</b><br/>
 * \tcat Unitaire · Text Input Field<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Retour arrière retire le dernier caractère du texte pré-rempli.
 * }
 */
TEST(TextInputFieldTest, RetourArriereRetireLeDernierCaractere) {
    hmi::InputState input;
    hmi::TextInputField field("Niveau");

    input.beginFrame();
    input.onKeyDown(hmi::Key::Backspace);
    field.update(input);

    EXPECT_EQ(field.text(), "Nivea");
}

/**
 * @brief Retour arrière retire un caractère accentué entier (UTF-8 multi-octets), pas un octet.
 * \castest{<b>Retour arrière retire un caractère accentué entier (UTF-8 multi-octets), pas un
 * octet.</b><br/>
 * \tcat Unitaire · Text Input Field<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Retour arrière retire un caractère accentué entier (UTF-8 multi-octets), pas un
 * octet.
 * }
 */
TEST(TextInputFieldTest, RetourArriereRetireUnCaractereAccentueEntier) {
    hmi::InputState input;
    hmi::TextInputField field;

    input.beginFrame();
    input.onCharTyped(static_cast<wchar_t>(0xE9));  // 'é'
    field.update(input);
    ASSERT_EQ(field.text(), "\xC3\xA9");  // 2 octets UTF-8, pas 1

    input.beginFrame();
    input.onKeyDown(hmi::Key::Backspace);
    field.update(input);

    EXPECT_TRUE(field.text().empty());
}

/**
 * @brief Sans validateur, Entrée confirme toujours la saisie courante.
 * \castest{<b>Sans validateur, Entrée confirme toujours la saisie courante.</b><br/>
 * \tcat Unitaire · Text Input Field<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Sans validateur, Entrée confirme toujours la saisie courante.
 * }
 */
TEST(TextInputFieldTest, EntreeConfirmeSansValidateur) {
    hmi::InputState input;
    hmi::TextInputField field("Niveau");

    input.beginFrame();
    input.onKeyDown(hmi::Key::Enter);
    field.update(input);

    EXPECT_TRUE(field.confirmed());
    EXPECT_EQ(field.text(), "Niveau");
}

/**
 * @brief Un validateur qui refuse le texte empêche la confirmation et marque un refus.
 * \castest{<b>Un validateur qui refuse le texte empêche la confirmation et marque un
 * refus.</b><br/>
 * \tcat Unitaire · Text Input Field<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Un validateur qui refuse le texte empêche la confirmation et marque un refus.
 * }
 */
TEST(TextInputFieldTest, EntreeRefuseeParLeValidateur) {
    hmi::InputState input;
    hmi::TextInputField field("", [](const std::string& text) { return !text.empty(); });

    input.beginFrame();
    input.onKeyDown(hmi::Key::Enter);
    field.update(input);

    EXPECT_FALSE(field.confirmed());
    EXPECT_TRUE(field.rejected());
}

/**
 * @brief Modifier le texte après un refus efface l'indicateur de refus.
 * \castest{<b>Modifier le texte après un refus efface l'indicateur de refus.</b><br/>
 * \tcat Unitaire · Text Input Field<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Modifier le texte après un refus efface l'indicateur de refus.
 * }
 */
TEST(TextInputFieldTest, EditionApresRefusEffaceLeRefus) {
    hmi::InputState input;
    hmi::TextInputField field("", [](const std::string& text) { return !text.empty(); });

    input.beginFrame();
    input.onKeyDown(hmi::Key::Enter);
    field.update(input);
    ASSERT_TRUE(field.rejected());

    input.beginFrame();
    input.onCharTyped(L'A');
    field.update(input);

    EXPECT_FALSE(field.rejected());
    EXPECT_EQ(field.text(), "A");
}

/**
 * @brief Échap annule la saisie sans modifier le texte du champ.
 * \castest{<b>Échap annule la saisie sans modifier le texte du champ.</b><br/>
 * \tcat Unitaire · Text Input Field<br/>
 * \tcrit Majeur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Échap annule la saisie sans modifier le texte du champ.
 * }
 */
TEST(TextInputFieldTest, EchapAnnule) {
    hmi::InputState input;
    hmi::TextInputField field("Niveau");

    input.beginFrame();
    input.onKeyDown(hmi::Key::Escape);
    field.update(input);

    EXPECT_TRUE(field.cancelled());
    EXPECT_EQ(field.text(), "Niveau");
}

/**
 * @brief Une fois confirmé, update() n'a plus aucun effet sur le champ.
 * \castest{<b>Une fois confirmé, update() n'a plus aucun effet sur le champ.</b><br/>
 * \tcat Unitaire · Text Input Field<br/>
 * \tcrit Mineur<br/>
 * \tetapes 1. Mettre en place le contexte du test (arrangement).<br/>2. Executer le scenario et
 * verifier les assertions.<br/>
 * \tattendu Une fois confirmé, update() n'a plus aucun effet sur le champ.
 * }
 */
TEST(TextInputFieldTest, SansEffetApresConfirmation) {
    hmi::InputState input;
    hmi::TextInputField field("Niveau");

    input.beginFrame();
    input.onKeyDown(hmi::Key::Enter);
    field.update(input);
    ASSERT_TRUE(field.confirmed());

    input.beginFrame();
    input.onCharTyped(L'X');
    field.update(input);

    EXPECT_EQ(field.text(), "Niveau");
}
