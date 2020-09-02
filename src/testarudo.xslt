<?xml version = "1.0" encoding = "UTF-8"?>
<xsl:stylesheet version = "1.0"
                xmlns:xsl = "http://www.w3.org/1999/XSL/Transform">

  <xsl:output method="text"/>

  <xsl:template match="interactive_test">
    <xsl:text>interactive test</xsl:text>
  </xsl:template>

  <xsl:template match="testudo">
    <xsl:apply-templates select="test"/>
  </xsl:template>

  <xsl:template match="test">
    <xsl:text>||~cartouche~||</xsl:text>
    <xsl:text>||~ident~||{||~normal~||</xsl:text>
    <xsl:text/><xsl:value-of select="@name"/>
    <xsl:text>||~ident~||}||~normal~|| </xsl:text>
    <xsl:text/>||~bold~||<xsl:value-of select="@title"/>||~normal~||<xsl:text/>
    <xsl:text>&#xa;</xsl:text>

    <xsl:apply-templates select="
        test
      | scope
      | step_id
      | text
      | multiline_text
      | declare
      | perform
      | try
      | catch
      | check_true
      | check_equal
      | check_approx
      | check_verify
      | uncaught_exception
      | show_value
      | show_multiline_value
      | separator
      "/>

    <xsl:text/>||~ident~||{<xsl:value-of select="@name"/>}<xsl:text/>
    <xsl:text> </xsl:text>
    <xsl:value-of select="@n_failed"/>/<xsl:value-of select="@n_total"/>
    <xsl:text> fail</xsl:text>
    <xsl:if test="@n_errors!='0'">
      <xsl:text>, </xsl:text>
      <xsl:value-of select="@n_errors"/>
      <xsl:text> err</xsl:text>
    </xsl:if>
    <xsl:text>||~normal~||</xsl:text>
    <xsl:choose>
      <xsl:when test="@n_errors='0'">
        <xsl:apply-templates select="@success"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:call-template name="flag_error"/>
      </xsl:otherwise>
    </xsl:choose>
    <xsl:text>&#xa;&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="scope">
    <xsl:text>||~ident~||#||~normal~|| ||~ident~||{||~normal~|| </xsl:text>
    <xsl:text>begin scope ||~ident~||"||~normal~|| </xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text> ||~ident~||"||~normal~||&#xa;</xsl:text>

    <xsl:apply-templates select="
        scope
      | step_id
      | text
      | multiline_text
      | declare
      | perform
      | try
      | catch
      | check_true
      | check_equal
      | check_approx
      | check_verify
      | uncaught_exception
      | show_value
      | show_multiline_value
      | separator
      "/>

    <xsl:text>||~ident~||#||~normal~|| ||~ident~||}||~normal~|| </xsl:text>
    <xsl:text>end scope ||~ident~||"||~normal~|| </xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text> ||~ident~||"||~normal~||&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="step_id">
    <xsl:text>||~ident~||[||~normal~|| </xsl:text>
    <xsl:value-of select="@id"/>
    <xsl:text> ||~ident~||]||~normal~||&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="text">
    <xsl:text>||~ident~||"||~normal~|| </xsl:text>
    <xsl:value-of select="."/>
    <xsl:text> ||~ident~||"||~normal~||&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="multiline_text">
    <xsl:text>||~multiline_begin~||</xsl:text>
    <xsl:value-of select="."/>
    <xsl:text>||~multiline_end~||&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="declare">
    <xsl:text>||~ident~||:||~normal~|| </xsl:text>
    <xsl:value-of select="."/>
    <xsl:text> ||~ident~||;||~normal~||&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="perform">
    <xsl:text>||~ident~||#||~normal~|| </xsl:text>
    <xsl:value-of select="."/>
    <xsl:text> ||~ident~||;||~normal~||&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="try">
    <xsl:text>||~ident~||&amp;||~normal~|| </xsl:text>
    <xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="catch">
    <xsl:if test="@exception_type!=''">
      <xsl:text> ||~ident~||&gt;||~normal~|| </xsl:text>
      <xsl:value-of select="@exception_type"/>
    </xsl:if>
    <xsl:text> ||~ident~||&gt; "||~normal~|| </xsl:text>
    <xsl:value-of select="."/>
    <xsl:text> ||~ident~||"||~normal~||</xsl:text>
    <xsl:apply-templates select="@success"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="check_true">
    <xsl:text>||~ident~||%||~normal~|| </xsl:text>
    <xsl:value-of select="expression1"/>
    <xsl:if test="@success='false'">
      <xsl:text> ||~ident~||:||~normal~|| false</xsl:text>
    </xsl:if>
    <xsl:apply-templates select="@success"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="check_equal">
    <xsl:text>||~ident~||%||~normal~|| </xsl:text>
    <xsl:value-of select="expression1"/>
    <xsl:text> ||~ident~||==||~normal~|| </xsl:text>
    <xsl:value-of select="expression2"/>
    <xsl:if test="@success='false'">
      <xsl:text> ||~ident~||:||~normal~|| </xsl:text>
      <xsl:value-of select="expression1/@value"/>
      <xsl:text> ||~ident~||==||~normal~|| </xsl:text>
      <xsl:value-of select="expression2/@value"/>
    </xsl:if>
    <xsl:apply-templates select="@success"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="check_approx">
    <xsl:text>||~ident~||%||~normal~|| </xsl:text>
    <xsl:value-of select="expression1"/>
    <xsl:text> ||~ident~||//||~normal~|| </xsl:text>
    <xsl:value-of select="expression2"/>
    <xsl:text> ||~ident~||+/-||~normal~|| </xsl:text>
    <xsl:value-of select="@max_error"/>
    <xsl:if test="@success='false'">
      <xsl:text> ||~ident~||:||~normal~|| </xsl:text>
      <xsl:value-of select="expression1/@value"/>
      <xsl:text> ||~ident~||//||~normal~|| </xsl:text>
      <xsl:value-of select="expression2/@value"/>
    </xsl:if>
    <xsl:apply-templates select="@success"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="check_verify">
    <xsl:text>||~ident~||%||~normal~|| </xsl:text>
    <xsl:value-of select="expression1"/>
    <xsl:text> ||~ident~||~||~normal~|| </xsl:text>
    <xsl:value-of select="predicate"/>
    <xsl:if test="@success='false'">
      <xsl:text> ||~ident~||:||~normal~|| </xsl:text>
      <xsl:value-of select="expression1/@value"/>
    </xsl:if>
    <xsl:apply-templates select="@success"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="uncaught_exception">
    <xsl:text>||~ident~||&gt; ||~normal~|| uncaught exception </xsl:text>
    <xsl:text>||~ident~||"||~normal~|| </xsl:text>
    <xsl:value-of select="."/>
    <xsl:text> ||~ident~||"||~normal~||</xsl:text>
    <xsl:call-template name="flag_error"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="show_value">
    <xsl:text>||~ident~||?||~normal~|| </xsl:text>
    <xsl:value-of select="expression1"/>
    <xsl:text> ||~ident~||:||~normal~|| </xsl:text>
    <xsl:value-of select="expression1/@value"/>
    <xsl:text>&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="show_multiline_value">
    <xsl:text>||~ident~||?||~normal~|| </xsl:text>
    <xsl:value-of select="expression1"/>
    <xsl:text> ||~ident~||:||~normal~||||~multiline_begin~||</xsl:text>
    <xsl:value-of select="expression1/@value"/>
    <xsl:text>||~multiline_end~||&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="separator">
    <xsl:text>||~lines~||</xsl:text>
    <xsl:text>||~full~||-</xsl:text>
    <xsl:text>||~normal~||&#xa;</xsl:text>
  </xsl:template>

  <xsl:template match="@success">
    <xsl:if test=".='true'">
      <xsl:text> ||~right~|| [||~successtag~|| OK ||~normal~||]</xsl:text>
    </xsl:if>
    <xsl:if test=".='false'">
      <xsl:text>||~failure~|| ||~right~||-||~normal~||</xsl:text>
      <xsl:text> [||~failuretag~||FAIL||~normal~||]</xsl:text>
    </xsl:if>
  </xsl:template>

  <xsl:template name="flag_error">
    <xsl:text>||~failure~|| ||~right~||-||~normal~||</xsl:text>
    <xsl:text> [||~error~||ERR-||~normal~||]</xsl:text>
  </xsl:template>

</xsl:stylesheet>
