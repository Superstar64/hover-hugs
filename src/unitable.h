#pragma once

/* Unicode general categories, listed in the same order as in the Unicode
 * standard -- this must be the same order as in Data.Char.
 */
enum {
    GENCAT_Lu,	/* Letter, Uppercase */
    GENCAT_Ll,	/* Letter, Lowercase */
    GENCAT_Lt,	/* Letter, Titlecase */
    GENCAT_Lm,	/* Letter, Modifier */
    GENCAT_Lo,	/* Letter, Other */
    GENCAT_Mn,	/* Mark, Non-Spacing */
    GENCAT_Mc,	/* Mark, Spacing Combining */
    GENCAT_Me,	/* Mark, Enclosing */
    GENCAT_Nd,	/* Number, Decimal */
    GENCAT_Nl,	/* Number, Letter */
    GENCAT_No,	/* Number, Other */
    GENCAT_Pc,	/* Punctuation, Connector */
    GENCAT_Pd,	/* Punctuation, Dash */
    GENCAT_Ps,	/* Punctuation, Open */
    GENCAT_Pe,	/* Punctuation, Close */
    GENCAT_Pi,	/* Punctuation, Initial quote */
    GENCAT_Pf,	/* Punctuation, Final quote */
    GENCAT_Po,	/* Punctuation, Other */
    GENCAT_Sm,	/* Symbol, Math */
    GENCAT_Sc,	/* Symbol, Currency */
    GENCAT_Sk,	/* Symbol, Modifier */
    GENCAT_So,	/* Symbol, Other */
    GENCAT_Zs,	/* Separator, Space */
    GENCAT_Zl,	/* Separator, Line */
    GENCAT_Zp,	/* Separator, Paragraph */
    GENCAT_Cc,	/* Other, Control */
    GENCAT_Cf,	/* Other, Format */
    GENCAT_Cs,	/* Other, Surrogate */
    GENCAT_Co,	/* Other, Private Use */
    GENCAT_Cn	/* Other, Not Assigned */
};

/* properties of a Unicode character */
struct CharProperties {
    int category;		/* Unicode general category */
    int upper_offset;		/* offset of the result of toUpper */
    int lower_offset;		/* offset of the result of toUpper */
    int title_offset;		/* offset of the result of toUpper */
};

/* A contiguous block of characters with the same properties */
struct CharBlock {
    int blk_start;		/* first character in the block            */
    int blk_length;		/* number of characters in the block       */
    const struct CharProperties *blk_properties;
				/* properties shared by these characters,  */
    				/* and possibly by other blocks too.       */
};

// todo, have `mkunitable` export this as a enum instead a runtime variable
extern const int NUM_BLOCKS;
extern const struct CharBlock char_block[];