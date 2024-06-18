#!/usr/bin/env python
# coding: utf-8

# Source: https://www.kaggle.com/code/metric/whoosh-utils

# In[ ]:


''' Utilities for generating and working with whoosh indexes.

You probably only need to use the following functions:
- create_index
- load_index
- get_searcher
- get_query_parser
- execute_query
- count_query_tokens
'''

import abc
import collections
import os
import re
import subprocess

if os.path.exists('/kaggle/working'):
    subprocess.run('pip install /kaggle/input/whoosh-wheel-2-7-4/Whoosh-2.7.4-py2.py3-none-any.whl', shell=True)

import whoosh.analysis
import whoosh.collectors
import whoosh.fields
import whoosh.index
import whoosh.matching
import whoosh.qparser
import whoosh.query
import whoosh.scoring
import whoosh.util.text


class UnmatchedParenthesesError(ValueError):
    pass


class InvalidIndexError(ValueError):
    pass


class QueryValidator:
    def validate_query(self, query, parent_has_index=False):
        index_allowlist = '|'.join(['ab', 'clm', 'cpc', 'detd', 'ti'])
        index_regexp = re.compile(r'\.(' + index_allowlist + ')\.$')
        inside_paren_regexp = re.compile(r'^(?P<left>\()(?P<inside>.+)(?P<right>\)(\.[a-z]+\.)?)$')

        def tokenize(text: str):
            open_paren_regexp = re.compile(r'\(')
            close_paren_regexp = re.compile(r'\)(\.[a-z]+\.)?')

            if len(open_paren_regexp.findall(text)) != len(close_paren_regexp.findall(text)):
                raise UnmatchedParenthesesError('Un-matched parentheses')

            dq = collections.deque(text)

            paren_level = 0
            curr_token = ''
            tokens = []

            while len(dq) > 0:
                curr_char = dq.popleft()
                curr_token += curr_char

                if curr_char == '(':
                    paren_level += 1
                elif curr_char == ')' and paren_level > 1:
                    paren_level -= 1
                elif curr_char == ')':
                    paren_level -= 1
                    next_char = dq.popleft() if len(dq) > 0 else None

                    if next_char == '.':
                        curr_token += next_char

                        while len(dq) > 0 and curr_token[-1] != ' ':
                            curr_token += dq.popleft()

                    tokens.append(curr_token)
                    curr_token = ''
                elif len(dq) == 0 or (curr_char == ' ' and paren_level == 0):
                    tokens.append(curr_token.strip())
                    curr_token = ''

            return tokens

        tokens = tokenize(query)

        for token in tokens:
            token = token.strip()
            num_indexes = len(index_regexp.findall(token))

            if token[-1] == '.' and (parent_has_index or num_indexes == 0):
                raise InvalidIndexError(f'Invalid index: {token}')

            search = inside_paren_regexp.match(token)

            if search is not None:
                self.validate_query(search.group('inside'), parent_has_index or num_indexes > 0)


class BrsPhrase(whoosh.query.Phrase):
    ''' Matches documents containing a given phrase.
    Very similar to whoosh.query.Phrase, but can consider order.
    '''

    def __init__(self, fieldname, words, slop=1, boost=1.0, char_ranges=None, ordered=False):
        '''
        fieldname: the field to search.
        words: a list of words (unicode strings) in the phrase.
        slop: the number of words allowed between each 'word' in the
            phrase; the default of 1 means the phrase must match exactly.
        boost: a boost factor that to apply to the raw score of
            documents matched by this query.
        char_ranges: if a Phrase object is created by the query parser,
            it will set this attribute to a list of (startchar, endchar) pairs
            corresponding to the words in the phrase.
        ordered: if the proximity search is ordered.
        '''

        self.fieldname = fieldname
        self.words = words
        self.slop = slop
        self.boost = boost
        self.char_ranges = char_ranges
        self.ordered = ordered

    def __eq__(self, other):
        return whoosh.query.Phrase.__eq__(self) and self.ordered == other.ordered

    def __repr__(self):
        return f'{self.__class__.__name__}({self.fieldname}, {self.words}, slop={self.slop}, boost=self.boost, ordered={self.ordered})'

    def __hash__(self):
        return whoosh.query.Phrase.__hash__(self) ^ hash(self.ordered)

    def normalize(self):
        if not self.words:
            return whoosh.query.NullQuery
        if len(self.words) == 1:
            t = whoosh.query.Term(self.fieldname, self.words[0])
            if self.char_ranges:
                t.startchar, t.endchar = self.char_ranges[0]
            return t

        words = [w for w in self.words if w is not None]
        return self.__class__(self.fieldname, words, slop=self.slop,
                              boost=self.boost, char_ranges=self.char_ranges, ordered=self.ordered)

    def matcher(self, searcher, context=None):
        fieldname = self.fieldname
        if fieldname not in searcher.schema:
            return whoosh.matching.NullMatcher()

        field = searcher.schema[fieldname]
        if not field.format or not field.format.supports('positions'):
            raise whoosh.query.QueryError('Phrase search: {self.fieldname} field has no positions')

        terms = []
        # Build a list of Term queries from the words in the phrase
        reader = searcher.reader()
        for word in self.words:
            try:
                word = field.to_bytes(word)
            except ValueError:
                return whoosh.matching.NullMatcher()

            if (fieldname, word) not in reader:
                # Shortcut the query if one of the words doesn't exist.
                return whoosh.matching.NullMatcher()
            terms.append(whoosh.query.Term(fieldname, word))

        # Create the equivalent SpanNear2 query from the terms
        q = whoosh.query.SpanNear2(terms, slop=self.slop, ordered=self.ordered, mindist=1)

        m = q.matcher(searcher, context)
        if self.boost != 1.0:
            m = whoosh.matching.WrappingMatcher(m, boost=self.boost)
        return m


class BrsProximityPlugin(whoosh.qparser.Plugin):
    # Expression used to find words if a schema isn't available
    wordexpr = whoosh.util.text.rcompile(r'\S+')

    class BrsProximityNode(whoosh.qparser.TextNode):
        def __init__(self, text, textstartchar, slop=1, ordered=False):
            whoosh.qparser.TextNode.__init__(self, text)
            self.textstartchar = textstartchar
            self.slop = slop
            self.ordered = ordered

        def r(self):
            return f'{self.__class__.__name__} {self.text}~{self.slop}'

        def apply(self, fn):
            return self.__class__(self.type, [fn(node) for node in self.nodes],
                                  slop=self.slop, boost=self.boost, ordered=self.ordered)

        def query(self, parser):
            text = self.text
            fieldname = self.fieldname or parser.fieldname

            # We want to process the text of the phrase into 'words' (tokens),
            # and also record the startchar and endchar of each word

            sc = self.textstartchar
            if parser.schema and fieldname in parser.schema:
                field = parser.schema[fieldname]
                if field.analyzer:
                    # We have a field with an analyzer, so use it to parse
                    # the phrase into tokens
                    tokens = field.tokenize(text, mode='query', chars=True)
                    words = []
                    char_ranges = []
                    for t in tokens:
                        words.append(t.text)
                        char_ranges.append((sc + t.startchar, sc + t.endchar))
                else:
                    # We have a field but it doesn't have a format object,
                    # for some reason (it's self-parsing?), so use process_text
                    # to get the texts (we won't know the start/end chars)
                    words = list(field.process_text(text, mode='query'))
                    char_ranges = [(None, None)] * len(words)
            else:
                # We're parsing without a schema, so just use the default
                # regular expression to break the text into words
                words = []
                char_ranges = []
                for match in BrsProximityPlugin.wordexpr.finditer(text):
                    words.append(match.group(0))
                    char_ranges.append((sc + match.start(), sc + match.end()))

            qclass = BrsPhrase
            q = qclass(fieldname, words, slop=self.slop, boost=self.boost,
                       char_ranges=char_ranges, ordered=self.ordered)
            return whoosh.qparser.attach(q, self)

    class BrsProximityTagger(whoosh.qparser.RegexTagger):
        def create(self, parser, match):
            text = match.group('t1') + ' ' + match.group('t2')
            textstartchar = 0
            slopstr = match.group('slop')
            slop = int(slopstr) if slopstr else 1
            ordered = match.group('op') == 'ADJ'
            return BrsProximityPlugin.BrsProximityNode(text, textstartchar, slop, ordered)

    def __init__(self, expr='(?P<t1>\\w+) (?P<op>ADJ|NEAR)(?P<slop>[1-9]?) (?P<t2>\\w+)'):
        self.expr = expr

    def taggers(self, parser):
        return [(self.BrsProximityTagger(self.expr), 0)]


class BrsWildcard(whoosh.query.PatternQuery, abc.ABC):
    '''BRS Wildcard conversion
    '''
    SPECIAL_CHARS = frozenset('*?$[')
    single_pattern = r'\?'
    multiple_pattern = r'\*|(\$(?!\d))'
    postfix_pattern = r'\$[1-9]{1}[0-9]?$'

    def __unicode__(self):
        return f'{self.fieldname}:{self.text}'

    __str__ = __unicode__

    # Convert BRS wildcard patterns to regex pattern
    def _get_pattern(self):
        text = self.text
        first_pattern = '^'

        curr_index = 0
        for m in re.finditer(self.single_pattern, text):
            first_pattern += text[curr_index:m.start()]
            first_pattern += '.'
            curr_index = m.end()

        if curr_index != len(text):
            first_pattern += text[curr_index:len(text)]

        second_pattern = ''
        curr_index = 0
        for m in re.finditer(self.multiple_pattern, first_pattern):
            second_pattern += first_pattern[curr_index:m.start()]
            second_pattern += '.+'
            curr_index = m.end()

        if curr_index != len(first_pattern):
            second_pattern += first_pattern[curr_index:len(first_pattern)]

        match = re.search(self.postfix_pattern, second_pattern)

        if match is not None:
            second_pattern = second_pattern[0:match.start()] + '.{1,' + match.group(0)[1:] + '}'
        return second_pattern + '$'

    def normalize(self):
        text = self.text

        # If no wildcard chars, convert to a normal term.
        if '*' not in text and '?' not in text and '$' not in text:
            return whoosh.query.Term(self.fieldname, self.text, boost=self.boost)
        # If the only wildcard char is an $/* at the end, convert to a Prefix query.
        elif ('?' not in text and (text.endswith('*') or text.endswith('$'))
              and (text.find('*') == len(text) - 1 or text.find('$') == len(text) - 1)):
            return whoosh.query.Prefix(self.fieldname, self.text[:-1], boost=self.boost)
        else:
            return self


class BrsWildcardPlugin(whoosh.qparser.TaggingPlugin):
    '''BRS Wildcard Plugin

    Looks for BRS wild card characters (*$?) in tokens and creates BRSWildcardNodes.
    '''

    expr = '(?P<text>[*$?])'

    def filters(self, parser):
        priority = 50
        return [(self.do_wildcards, priority)]

    def do_wildcards(self, parser, group):
        i = 0
        while i < len(group):
            node = group[i]
            if isinstance(node, self.BrsWildcardNode):
                if i < len(group) - 1 and group[i + 1].is_text():
                    nextnode = group.pop(i + 1)
                    node.text += nextnode.text
                if i > 0 and group[i - 1].is_text():
                    prevnode = group.pop(i - 1)
                    node.text = prevnode.text + node.text
                else:
                    i += 1
            else:
                if isinstance(node, whoosh.qparser.GroupNode):
                    self.do_wildcards(parser, node)
                i += 1

        return group

    class BrsWildcardNode(whoosh.qparser.TextNode):
        qclass = BrsWildcard

        def r(self):
            return f'BrsWildcard {self.text}'

    nodetype = BrsWildcardNode


class BrsXorGroup(whoosh.qparser.GroupNode):
    merging = True

    class BrsXor(whoosh.query.CompoundQuery, abc.ABC):
        '''XOR operator

        A XOR B = (A OR B) AND (NOT (A AND B))
        '''

        JOINT = ' XOR '
        intersect_merge = True

        def __init__(self, subqueries, boost=1.0):
            left_or = whoosh.query.Or(subqueries)
            right_and = whoosh.query.And(subqueries)
            right_not = whoosh.query.Not(right_and)
            self.full = whoosh.query.And([left_or, right_not])
            self.subqueries = subqueries
            self.boost = boost

        def __unicode__(self):
            return self.full.__unicode__()

        __str__ = __unicode__

        def normalize(self):
            return self.full.normalize()

        def requires(self):
            return self.full.requires()

        def _matcher(self, subs, searcher, context):
            return self.full._matcher(subs, searcher, context)

    qclass = BrsXor


NUMBER_REGEX = re.compile(r'^(\d+|\d{1,3}(,\d{3})*)(\.\d+)?$')

class NumberFilter(whoosh.analysis.Filter):
    def __call__(self, tokens):
        for t in tokens:
            if not NUMBER_REGEX.match(t.text):
                yield t


def _define_uspto_whoosh_schema():
    BRS_STOPWORDS = ['an', 'are', 'by', 'for', 'if', 'into', 'is', 'no', 'not', 'of', 'on', 'such',
                     'that', 'the', 'their', 'then', 'there', 'these', 'they', 'this', 'to', 'was', 'will']

    # Prevent both stopwords and numbers from ever being indexed.
    custom_analyzer = whoosh.analysis.StandardAnalyzer(stoplist=BRS_STOPWORDS) | NumberFilter()
    # Schema fields match a subset of the USPTO list of searchable indexes:
    # https://ppubs.uspto.gov/pubwebapp/static/pages/searchable-indexes.html
    schema = whoosh.fields.Schema(
        id=whoosh.fields.ID(stored=True),
        ti=whoosh.fields.TEXT(analyzer=custom_analyzer, stored=False),
        ab=whoosh.fields.TEXT(analyzer=custom_analyzer, stored=False),
        clm=whoosh.fields.TEXT(analyzer=custom_analyzer, stored=False),
        detd=whoosh.fields.TEXT(analyzer=custom_analyzer, stored=False),
        cpc=whoosh.fields.KEYWORD(stored=False, scorable=True)
    )
    return schema


def create_index(output_dir, documents, limitmb=5_000):
    '''
    Args:
        output_dir: (str) the path where the index should be created.

        documents: (list[{str: str}]) an iterable containing one dictionary per patent.

        limitmb: (int) the maximum memory used during indexing. See
        https://whoosh.readthedocs.io/en/latest/batch.html#the-limitmb-parameter
    '''
    ix = whoosh.index.create_in(output_dir, schema=_define_uspto_whoosh_schema())
    writer = ix.writer(proces=os.cpu_count(), multisegment=True, limitmb=limitmb)

    for document in documents:
        writer.add_document(
            id=document['publication_number'],
            ti=document['title'],
            ab=document['abstract'],
            clm=document['claims'],
            detd=document['description'],
            cpc=document['cpc']
        )
    writer.commit(optimize=True)
    ix.close()


def get_query_parser():
    schema = _define_uspto_whoosh_schema()
    qp = whoosh.qparser.MultifieldParser(['ti', 'ab', 'clm', 'detd', 'cpc'], schema=schema)

    qp.remove_plugin_class(whoosh.qparser.BoostPlugin)
    qp.remove_plugin_class(whoosh.qparser.CopyFieldPlugin)
    qp.remove_plugin_class(whoosh.qparser.FieldAliasPlugin)
    qp.remove_plugin_class(whoosh.qparser.GtLtPlugin)
    qp.remove_plugin_class(whoosh.qparser.PlusMinusPlugin)
    qp.remove_plugin_class(whoosh.qparser.PrefixPlugin)
    qp.remove_plugin_class(whoosh.qparser.RangePlugin)
    qp.remove_plugin_class(whoosh.qparser.RegexPlugin)
    qp.remove_plugin_class(whoosh.qparser.WildcardPlugin)

    qp.add_plugin(BrsProximityPlugin)
    qp.add_plugin(BrsWildcardPlugin)

    xor_tagger = whoosh.qparser.OperatorsPlugin.OpTagger(' XOR ', BrsXorGroup, whoosh.qparser.InfixOperator)
    qp.replace_plugin(whoosh.qparser.OperatorsPlugin([(xor_tagger, 0)], Require=None, AndNot=None, AndMaybe=None))
    return qp


def load_index(index_dir):
    return whoosh.index.open_dir(index_dir, schema=_define_uspto_whoosh_schema())


def get_searcher(whoosh_index):
    return whoosh_index.searcher(weighting=whoosh.scoring.TF_IDF())


def execute_query(query: str, qp, searcher, results_limit=50) -> list:
    if len(query) > 10_000:
        raise ValueError('Query length at exceeds 10,000 characters.')
    if 'id:' in query:
        raise ValueError('Searching for specific patent IDs is banned.')

    to_search = qp.parse(query)
    results = searcher.search(to_search, limit=results_limit)
    results = [x['id'] for x in results]
    assert len(results) <= results_limit
    return results


def count_query_tokens(query: str):
    # Count the number of tokens in the query.
    # Treat entries like "cpc:AO1B33/00" as a single token.
    return len([i for i in re.split('[\s+()]', query) if i])
