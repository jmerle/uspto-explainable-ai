#include <filesystem>
#include <string>
#include <vector>

#include <ankerl/unordered_dense.h>
#include <gtest/gtest.h>

#include <uspto/files.h>
#include <uspto/patents.h>

TEST(patents, extractTextTokens) {
    TemporaryDirectory temporaryDirectory;

    PatentWriter writer(temporaryDirectory.path);

    EXPECT_EQ(writer.extractTextTokens(""), std::vector<std::string>({}));
    EXPECT_EQ(writer.extractTextTokens("Hello, world!"), std::vector<std::string>({"hello", "world"}));
    EXPECT_EQ(writer.extractTextTokens("a b c ab bc abc"), std::vector<std::string>({"ab", "bc", "abc"}));
    EXPECT_EQ(writer.extractTextTokens("a.thing"), std::vector<std::string>({"a.thing"}));
    EXPECT_EQ(writer.extractTextTokens("a. thing"), std::vector<std::string>({"thing"}));
    EXPECT_EQ(writer.extractTextTokens("a.thing.a"), std::vector<std::string>({"a.thing.a"}));
    EXPECT_EQ(writer.extractTextTokens("a. thing. a"), std::vector<std::string>({"thing"}));
    EXPECT_EQ(writer.extractTextTokens("A.THiNG.A"), std::vector<std::string>({"a.thing.a"}));
    EXPECT_EQ(writer.extractTextTokens("A.THiNG42.A"), std::vector<std::string>({"a.thing42.a"}));
    EXPECT_EQ(writer.extractTextTokens("There are 7 things"), std::vector<std::string>({"things"}));
    EXPECT_EQ(writer.extractTextTokens("1.2.3.4.5"), std::vector<std::string>({"1.2.3.4.5"}));
    EXPECT_EQ(writer.extractTextTokens("123456"), std::vector<std::string>({}));
    EXPECT_EQ(writer.extractTextTokens("123.456"), std::vector<std::string>({}));
    EXPECT_EQ(writer.extractTextTokens("123.456."), std::vector<std::string>({}));
    EXPECT_EQ(writer.extractTextTokens("123.456.a"), std::vector<std::string>({"123.456.a"}));
    EXPECT_EQ(writer.extractTextTokens("123.456.7"), std::vector<std::string>({"123.456.7"}));
    EXPECT_EQ(writer.extractTextTokens("123.456.78"), std::vector<std::string>({"123.456.78"}));
    EXPECT_EQ(writer.extractTextTokens("123.456.789"), std::vector<std::string>({"123.456.789"}));
    EXPECT_EQ(writer.extractTextTokens("1.234.56"), std::vector<std::string>({"1.234.56"}));
    EXPECT_EQ(writer.extractTextTokens("1.234.56"), std::vector<std::string>({"1.234.56"}));
    EXPECT_EQ(writer.extractTextTokens("1,23456"), std::vector<std::string>({}));
    EXPECT_EQ(writer.extractTextTokens("123456.789"), std::vector<std::string>({}));
    EXPECT_EQ(writer.extractTextTokens("1.23 cm"), std::vector<std::string>({"cm"}));
    EXPECT_EQ(writer.extractTextTokens("1.23cm"), std::vector<std::string>({"1.23cm"}));
    EXPECT_EQ(writer.extractTextTokens("123,456,789.123"), std::vector<std::string>({}));
    EXPECT_EQ(writer.extractTextTokens("should. be"), std::vector<std::string>({"should", "be"}));
    EXPECT_EQ(writer.extractTextTokens("should .be"), std::vector<std::string>({"should", "be"}));

    EXPECT_EQ(
        writer.extractTextTokens(
            R"DESCRIPTION(
The present invention comprises a new and distinct cultivar of rose plant of the Hybrid Tea Rose Class, hereinafter referred to by the cultivar name `Schomi`.
 `Schomi` is a product of a planned breeding program having the objective of creating new rose cultivars which have high yields, longer vase life, longer stems, reduced numbers of thorns and mildew resistance.
 `Schomi` was originated from a hybridization made by Petrus N.J. Schreurs in a controlled breeding program in De Kwakel, The Netherlands, in 1988. The female parent was the variety `Huubda` (Dutch Plant Breeder&#39;s Rights No. NL. 9119. The male parent was an unnamed Piet Schreurs De Kwakel B.V. proprietary seedling.
 `Schomi` was discovered and selected as one flowering plant within the progeny of the stated parentage by Petrus J.N. Schreurs, in February, 1989, in a controlled environment in De Kwakel, The Netherlands. Asexual reproduction of the new cultivar by budding and cuttings, as performed by Petrus J.N. Schreurs in De Kwakel, The Netherlands, has demonstrated that the combination of characteristics, as herein disclosed for `Schomi`, are firmly fixed through successive generations of asexual reproduction.
 `Schomi` has not been observed under all possible environmental conditions. Phenotype may vary significantly with variations in environment, such as temperature, light intensity, and daylength, without any change in genotype.
 The following traits have been repeatedly observed and are determined to be basic characteristics of `Schomi`, which in combination, distinguish this rose as a new and distinct cultivar:
 1. Double-type flower with 35-40 petals per flower.
 2. Petal color on upper side of R.H.S. 55D and on underside of R.H.S. 38A.
 3. Length of main stems approximately 90 cm.
 4. Length of canes approximately 30 cm.
 Of the many commercial cultivars known to the present inventor, the most similar in comparison to `Schomi` is `Jacakor`. `Schomi` has a petal color on the upper side of R.H.S. 55D and on the underside of R.H.S. 38A. `Schomi` has a bud form that is wide and narrow at the tip. In contrast, `Jacakor` has a petal color of lilac purple a higher yield, shorter stems, and a bud form that is narrower and less spherical than `Schomi`.
 The parental cultivar `Huubda` and `Schomi` are comparable in height and width. However, `Huubda` has a different pink color, stronger fragrance, and higher yield than `Schomi`.


 The accompanying color photographic drawing shows the typical inflorescence and foliage characteristics of `Schomi`, with colors as nearly true as possible with illustrations of this type.
 Sheet 1 is a side view of the plant showing leaves, stems and flowers.


 In the following description, color references are made to The Royal Horticultural Society (R.H.S.) Colour Chart. The color values were determined between 10:00 a.m. and 4:00 p.m. under 1.550 J/cma light intensity in De Kwakel, The Netherlands.
 Classification:
 Botanical.--Rosa Hybrida, cv. `Schomi`.
 Commercial.--Hybrid Tea Rose.
 Plant:
 Habit.--Bush rose.
 Growth.--Average vigor.
 Average stem length.--90 cm.
 Average cane length.--30 cm.
 Thorns and prickles.--Young plants have few or no thorns and the number of thorns increases as the plant matures. The thorns measure approximately 0.7 cm in length. The color is yellowish green with a shade of red, R.H.S. 179 A, caused by production of anthocyanin.
 Plant width at maturity.--Approximately 60 to 70 cm.
 Plant height at maturity.--Approximately 120 to 130 cm.
 Foliage.--Shape leaflets: Oval. Leaflet size: Single: 7-8 cm in length and 4.5-6 cm in width. Compound: Most often 3 leaflets form the compound leaf which is about 21 cm in length and about 15 cm in width. Glossiness: Absent. Appearance: Lightly serrated. Color (top side): Medium green (R.H.S. 137A) with dark red shades caused by production of anthocyanin, which appears at the leaf periphery. Color (bottom side) Light green (R.H.S. 147B). Stipules: Average length is 8 mm. Other foliage characteristics: The leaves curl toward bottom side of the leaves.
 Flowers:
 Borne.--Upright.
 Number of flowers per stem.--One.
 Buds.--Peduncle length: Average length is 10-20 cm. Peduncle surface: Smooth with color of green with red shades (R.H.S. 146A). Peduncle strength: Thick and strong. Bud shape: Wide with a tip. Bud size: Average width is 4.5 cm and average length is 5 cm. Bud color: Red, R.H.S. 48 A, variegating to green, R.H.S. 144 B, with a red shade. Receptacle color: Light green with red shades, R.H.S. 48D. Receptacle shape: Wide oval. Receptacle surface: Smooth. Sepal color: Light green, R.H.S. 144 B, variegated with red, R.H.S. 47 B. Sepal appearance: Serrated edges. Sepal surface texture: Pubescence present. Blooms: Size: Diameter of 11-12 cm. Petalage: Number: Double-type (35-40 per flower). Length: 5-6 cm. Texture: Smooth without glossiness. Shape: Broad oval. Hips: Absent. Color: Upperside: White Group R.H.S. 55D. Underside: R.H.S. 38A.
 Fragrance.--Very weak.
 Reproductive organs.--Stamens: Average number 140 to 160. Pistils: Average number 150 to 170. Anthers: R.H.S. 10B. Filaments: White (R.H.S. 4D). Pollen: Yellow (R.H.S. 15A). Styles: Yellow (R.H.S. 14C). Stigma: Yellow white. Ovaries: White (R.H.S. 4D).
 Other characteristics:
 Disease resistance.--Average, no significant resistance to any specific pathogen has been observed.
)DESCRIPTION"),
        std::vector<std::string>({
            "present", "invention", "comprises", "new", "and", "distinct", "cultivar", "rose", "plant", "hybrid", "tea",
            "rose", "class", "hereinafter", "referred", "cultivar", "name", "schomi", "schomi", "product", "planned",
            "breeding", "program", "having", "objective", "creating", "new", "rose", "cultivars", "which", "have",
            "high", "yields", "longer", "vase", "life", "longer", "stems", "reduced", "numbers", "thorns", "and",
            "mildew", "resistance", "schomi", "originated", "from", "hybridization", "made", "petrus", "n.j", "schreurs"
            , "in", "controlled", "breeding", "program", "in", "de", "kwakel", "netherlands", "in", "female", "parent",
            "variety", "huubda", "dutch", "plant", "breeder", "rights", "nl", "male", "parent", "unnamed", "piet",
            "schreurs", "de", "kwakel", "b.v", "proprietary", "seedling", "schomi", "discovered", "and", "selected",
            "as", "one", "flowering", "plant", "within", "progeny", "stated", "parentage", "petrus", "j.n", "schreurs",
            "in", "february", "in", "controlled", "environment", "in", "de", "kwakel", "netherlands", "asexual",
            "reproduction", "new", "cultivar", "budding", "and", "cuttings", "as", "performed", "petrus", "j.n",
            "schreurs", "in", "de", "kwakel", "netherlands", "has", "demonstrated", "combination", "characteristics",
            "as", "herein", "disclosed", "schomi", "firmly", "fixed", "through", "successive", "generations", "asexual",
            "reproduction", "schomi", "has", "been", "observed", "under", "all", "possible", "environmental",
            "conditions", "phenotype", "may", "vary", "significantly", "with", "variations", "in", "environment", "as",
            "temperature", "light", "intensity", "and", "daylength", "without", "any", "change", "in", "genotype",
            "following", "traits", "have", "been", "repeatedly", "observed", "and", "determined", "be", "basic",
            "characteristics", "schomi", "which", "in", "combination", "distinguish", "rose", "as", "new", "and",
            "distinct", "cultivar", "double", "type", "flower", "with", "petals", "per", "flower", "petal", "color",
            "upper", "side", "r.h.s", "55d", "and", "underside", "r.h.s", "38a", "length", "main", "stems",
            "approximately", "cm", "length", "canes", "approximately", "cm", "many", "commercial", "cultivars", "known",
            "present", "inventor", "most", "similar", "in", "comparison", "schomi", "jacakor", "schomi", "has", "petal",
            "color", "upper", "side", "r.h.s", "55d", "and", "underside", "r.h.s", "38a", "schomi", "has", "bud", "form"
            , "wide", "and", "narrow", "at", "tip", "in", "contrast", "jacakor", "has", "petal", "color", "lilac",
            "purple", "higher", "yield", "shorter", "stems", "and", "bud", "form", "narrower", "and", "less",
            "spherical", "than", "schomi", "parental", "cultivar", "huubda", "and", "schomi", "comparable", "in",
            "height", "and", "width", "however", "huubda", "has", "different", "pink", "color", "stronger", "fragrance",
            "and", "higher", "yield", "than", "schomi", "accompanying", "color", "photographic", "drawing", "shows",
            "typical", "inflorescence", "and", "foliage", "characteristics", "schomi", "with", "colors", "as", "nearly",
            "true", "as", "possible", "with", "illustrations", "type", "sheet", "side", "view", "plant", "showing",
            "leaves", "stems", "and", "flowers", "in", "following", "description", "color", "references", "made",
            "royal", "horticultural", "society", "r.h.s", "colour", "chart", "color", "values", "were", "determined",
            "between", "a.m", "and", "p.m", "under", "cma", "light", "intensity", "in", "de", "kwakel", "netherlands",
            "classification", "botanical", "rosa", "hybrida", "cv", "schomi", "commercial", "hybrid", "tea", "rose",
            "plant", "habit", "bush", "rose", "growth", "average", "vigor", "average", "stem", "length", "cm", "average"
            , "cane", "length", "cm", "thorns", "and", "prickles", "young", "plants", "have", "few", "or", "thorns",
            "and", "number", "thorns", "increases", "as", "plant", "matures", "thorns", "measure", "approximately", "cm"
            , "in", "length", "color", "yellowish", "green", "with", "shade", "red", "r.h.s", "caused", "production",
            "anthocyanin", "plant", "width", "at", "maturity", "approximately", "cm", "plant", "height", "at",
            "maturity", "approximately", "cm", "foliage", "shape", "leaflets", "oval", "leaflet", "size", "single", "cm"
            , "in", "length", "and", "cm", "in", "width", "compound", "most", "often", "leaflets", "form", "compound",
            "leaf", "which", "about", "cm", "in", "length", "and", "about", "cm", "in", "width", "glossiness", "absent",
            "appearance", "lightly", "serrated", "color", "top", "side", "medium", "green", "r.h.s", "137a", "with",
            "dark", "red", "shades", "caused", "production", "anthocyanin", "which", "appears", "at", "leaf",
            "periphery", "color", "bottom", "side", "light", "green", "r.h.s", "147b", "stipules", "average", "length",
            "mm", "other", "foliage", "characteristics", "leaves", "curl", "toward", "bottom", "side", "leaves",
            "flowers", "borne", "upright", "number", "flowers", "per", "stem", "one", "buds", "peduncle", "length",
            "average", "length", "cm", "peduncle", "surface", "smooth", "with", "color", "green", "with", "red",
            "shades", "r.h.s", "146a", "peduncle", "strength", "thick", "and", "strong", "bud", "shape", "wide", "with",
            "tip", "bud", "size", "average", "width", "cm", "and", "average", "length", "cm", "bud", "color", "red",
            "r.h.s", "variegating", "green", "r.h.s", "with", "red", "shade", "receptacle", "color", "light", "green",
            "with", "red", "shades", "r.h.s", "48d", "receptacle", "shape", "wide", "oval", "receptacle", "surface",
            "smooth", "sepal", "color", "light", "green", "r.h.s", "variegated", "with", "red", "r.h.s", "sepal",
            "appearance", "serrated", "edges", "sepal", "surface", "texture", "pubescence", "present", "blooms", "size",
            "diameter", "cm", "petalage", "number", "double", "type", "per", "flower", "length", "cm", "texture",
            "smooth", "without", "glossiness", "shape", "broad", "oval", "hips", "absent", "color", "upperside", "white"
            , "group", "r.h.s", "55d", "underside", "r.h.s", "38a", "fragrance", "very", "weak", "reproductive",
            "organs", "stamens", "average", "number", "pistils", "average", "number", "anthers", "r.h.s", "10b",
            "filaments", "white", "r.h.s", "4d", "pollen", "yellow", "r.h.s", "15a", "styles", "yellow", "r.h.s", "14c",
            "stigma", "yellow", "white", "ovaries", "white", "r.h.s", "4d", "other", "characteristics", "disease",
            "resistance", "average", "significant", "resistance", "any", "specific", "pathogen", "has", "been",
            "observed"}));

    EXPECT_EQ(
        writer.extractTextTokens(
            R"DESCRIPTION(
J. ANDERSON.
 Horse SunBonnet.
 No.-100,000. Patented Feb. 22, 1870.
  IHHII N PETERS. Pmwm mn wumn m. n, a
 new emu;
 JOHN ANDERSON, OF BROOKLYN, NEW-.YORK.
  Letters Patent No. 100,000, dated February 22, 1870.
 IMPROVED SITN-BOINNE.I. FOR HORSES.
 The Schedule referred. to in these Letters Patent and maklng part of the same.
 To all whom it may concern Be it known that I, Join Annnnson, of&#39;Brooklyn, Kings County, and State of New York, have invented a new and useful Improvement in Shields or Sun-Bonnets for Horses, and do hereby declare that the following is a general description thereotireference being bad to the accompanying drawings making&#39;part of this specification.
  As the object of this invention is the shielding of the horses head from the rays of the&#39;sun, the present applicant had necessarily to provide a space between the head and the protecting shield or bonnet for the constant passage of a current of air. It was also requisite to have the bonnet or shield not only easily attached and detached, but that it should .be securely held in position in a manner agreeable to the animal. Moreover, it was equally essential that the contrivance for fastening on the bonnet shouldnot so pull upon the devices for preserving the air space that the said space should be too much reduced. Hence the elastic side attachments &#39;andthe supporting transverse straps or hands shown in the drawings are of far different tensile strengths, the former being of ample strength for securing the bonnet in place, but. far too weak to so strain upon the transverse or supporting straps as to too far reduce the&#39;air space between the&#39; horses head and the shield, upon which the utility of latter depends.
 In the said drawings Figure 1 represents my improved shield placed on top of the head showing its elasticsea&#39;ts, the apertures for the ears, and the connection with thehridle, and forming with a bridle-the head gear of the horse;
 Figure 2 an under-side view; and
 Figure 3 a vertical section of said shield.
 In the said drawings-- A indicates the head and neck-of a horse, and
 B the bridle.
  G is the shield composed of a wire, wooden, or&#39;other suitable frame work, I), which is covered with cloth, E,&#39;or other suitable material.
  F F are the apertures in the shield through which the horses ears protrude.
  G G are the bearings which rest on the head, supporting the shield, and leaving space H for a current the horses head, projecting in front over the forehead,
 and back over and covering the junction of the cerebellum or little brain with the spinal marrowwhich lies near the surface, and is quickly efiectedby the heat of the sun, and in my experience I have found it to be etl&#39;icient in protecting horses from sun-stroke I am aware that Letters Patent of the United States were granted to me July 6, 1869, for an improved shield for protecting horses from sun-stroke; but I found that, in the absence of the bearings G G, the shield or bonnet was apt to descend to the head, leaving no air space between the bonnet and the head,
 and besides this the rigid fastening tended to throw the bonnet back and forth on the horses face at each movement of the head. These defects suggested to me my present improvement.
 Having described my invention,
 resting on the horses head before and behind the ears, and securing a constant current of air, as shown and 2. The elastic .or other fastenings I I, when attached to the bridle with a view to facility of attachment and removal, as explained in preamble and specification.
 In testimony whereof, I have hereunto set my sig- I nature this 28th day of July, A. D. 1869.
 - JOHN ANDERSON.
 Witnesses: I
 WM. H. OAMMEYER, SAMUEL LEWIS.
)DESCRIPTION"),
        std::vector<std::string>({
            "anderson", "horse", "sunbonnet", "patented", "feb", "ihhii", "peters", "pmwm", "mn", "wumn", "new", "emu",
            "john", "anderson", "brooklyn", "new", "york", "letters", "patent", "dated", "february", "improved", "sitn",
            "boinne.i", "horses", "schedule", "referred", "in", "letters", "patent", "and", "maklng", "part", "same",
            "all", "whom", "it", "may", "concern", "be", "it", "known", "join", "annnnson", "brooklyn", "kings",
            "county", "and", "state", "new", "york", "have", "invented", "new", "and", "useful", "improvement", "in",
            "shields", "or", "sun", "bonnets", "horses", "and", "do", "hereby", "declare", "following", "general",
            "description", "thereotireference", "being", "bad", "accompanying", "drawings", "making", "part",
            "specification", "as", "object", "invention", "shielding", "horses", "head", "from", "rays", "sun",
            "present", "applicant", "had", "necessarily", "provide", "space", "between", "head", "and", "protecting",
            "shield", "or", "bonnet", "constant", "passage", "current", "air", "it", "also", "requisite", "have",
            "bonnet", "or", "shield", "only", "easily", "attached", "and", "detached", "but", "it", "should", "be",
            "securely", "held", "in", "position", "in", "manner", "agreeable", "animal", "moreover", "it", "equally",
            "essential", "contrivance", "fastening", "bonnet", "shouldnot", "so", "pull", "upon", "devices",
            "preserving", "air", "space", "said", "space", "should", "be", "too", "much", "reduced", "hence", "elastic",
            "side", "attachments", "andthe", "supporting", "transverse", "straps", "or", "hands", "shown", "in",
            "drawings", "far", "different", "tensile", "strengths", "former", "being", "ample", "strength", "securing",
            "bonnet", "in", "place", "but", "far", "too", "weak", "so", "strain", "upon", "transverse", "or",
            "supporting", "straps", "as", "too", "far", "reduce", "air", "space", "between", "horses", "head", "and",
            "shield", "upon", "which", "utility", "latter", "depends", "in", "said", "drawings", "figure", "represents",
            "my", "improved", "shield", "placed", "top", "head", "showing", "its", "elasticsea", "ts", "apertures",
            "ears", "and", "connection", "with", "thehridle", "and", "forming", "with", "bridle", "head", "gear",
            "horse", "figure", "under", "side", "view", "and", "figure", "vertical", "section", "said", "shield", "in",
            "said", "drawings", "indicates", "head", "and", "neck", "horse", "and", "bridle", "shield", "composed",
            "wire", "wooden", "or", "other", "suitable", "frame", "work", "which", "covered", "with", "cloth", "or",
            "other", "suitable", "material", "apertures", "in", "shield", "through", "which", "horses", "ears",
            "protrude", "bearings", "which", "rest", "head", "supporting", "shield", "and", "leaving", "space",
            "current", "horses", "head", "projecting", "in", "front", "over", "forehead", "and", "back", "over", "and",
            "covering", "junction", "cerebellum", "or", "little", "brain", "with", "spinal", "marrowwhich", "lies",
            "near", "surface", "and", "quickly", "efiectedby", "heat", "sun", "and", "in", "my", "experience", "have",
            "found", "it", "be", "etl", "icient", "in", "protecting", "horses", "from", "sun", "stroke", "am", "aware",
            "letters", "patent", "united", "states", "were", "granted", "me", "july", "improved", "shield", "protecting"
            , "horses", "from", "sun", "stroke", "but", "found", "in", "absence", "bearings", "shield", "or", "bonnet",
            "apt", "descend", "head", "leaving", "air", "space", "between", "bonnet", "and", "head", "and", "besides",
            "rigid", "fastening", "tended", "throw", "bonnet", "back", "and", "forth", "horses", "face", "at", "each",
            "movement", "head", "defects", "suggested", "me", "my", "present", "improvement", "having", "described",
            "my", "invention", "resting", "horses", "head", "before", "and", "behind", "ears", "and", "securing",
            "constant", "current", "air", "as", "shown", "and", "elastic", "or", "other", "fastenings", "when",
            "attached", "bridle", "with", "view", "facility", "attachment", "and", "removal", "as", "explained", "in",
            "preamble", "and", "specification", "in", "testimony", "whereof", "have", "hereunto", "set", "my", "sig",
            "nature", "28th", "day", "july", "john", "anderson", "witnesses", "wm", "oammeyer", "samuel", "lewis"}));
}

TEST(patents, writeAndReadPatentData) {
    TemporaryDirectory temporaryDirectory;

    {
        PatentWriter writer(temporaryDirectory.path);

        writer.writePatent(
            "US-1-A",
            {"code1", "code2", "code3"},
            "This is a title title",
            "This is an abstract",
            "These are some claims",
            "This is a description");
    }

    PatentReader reader(temporaryDirectory.path);

    auto allTerms = reader.readTermsWithCounts(
        "US-1-A",
        TermCategory::Title
        | TermCategory::Abstract
        | TermCategory::Claims
        | TermCategory::Description
        | TermCategory::Cpc);
    EXPECT_EQ(
        allTerms,
        (ankerl::unordered_dense::map<std::string, std::uint16_t>({
            {"cpc:code1", 1},
            {"cpc:code2", 1},
            {"cpc:code3", 1},
            {"ti:title", 2},
            {"ab:abstract", 1},
            {"clm:some", 1},
            {"clm:claims", 1},
            {"detd:description", 1}})));

    auto titleTerms = reader.readTerms("US-1-A", TermCategory::Title);
    EXPECT_EQ(titleTerms, std::vector<std::string>({"ti:title"}));
}
